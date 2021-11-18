
#include "line_graph.h"

#include <geometry/line_intersection.h>
#include <util.h>

namespace omg {

LineGraph::LineGraph(const HEPolygon& poly) {
    // convert from HEPolygon to LineGraph

    const std::size_t num = poly.numVertices();
    std::size_t idx = 0;
    for (HEPolygon::VertexHandle v : poly.verticesOrdered()) {

        addVertex(poly.point(v));

        addEdge(idx, (idx + 1) % num);
        idx++;
    }
}

LineGraph LineGraph::createRectangle(const AxisAlignedBoundingBox& aabb) {
    LineGraph lg;

    VertexHandle v0 = lg.addVertex(vec2_t(aabb.min[0], aabb.min[1]));
    VertexHandle v1 = lg.addVertex(vec2_t(aabb.max[0], aabb.min[1]));
    VertexHandle v2 = lg.addVertex(vec2_t(aabb.max[0], aabb.max[1]));
    VertexHandle v3 = lg.addVertex(vec2_t(aabb.min[0], aabb.max[1]));

    lg.addEdge(v0, v1);
    lg.addEdge(v1, v2);
    lg.addEdge(v2, v3);
    lg.addEdge(v3, v0);

    return lg;
}

LineGraph LineGraph::combinePolygons(const std::vector<HEPolygon>& polys) {
    LineGraph lg;
    std::size_t offset = 0;

    // combine all HEPolygons into one LineGraph
    for (const HEPolygon& p : polys) {

        LineGraph l(p);
        for (const vec2_t& v : l.getPoints()) {
            lg.addVertex(v);
        }
        for (const Edge& e : l.getEdges()) {
            lg.addEdge(e.first + offset, e.second + offset);
        }

        offset += l.getPoints().size();
    }

    return lg;
}

LineGraph::VertexHandle LineGraph::addVertex(const vec2_t& v) {
    const VertexHandle idx = points.size();

    points.push_back(v);

    return idx;
}

LineGraph::EdgeHandle LineGraph::addEdge(VertexHandle v1, VertexHandle v2) {
    const EdgeHandle idx = edges.size();

    edges.push_back({v1, v2});

    return idx;
}

void LineGraph::removeEdgesByIndex(std::vector<EdgeHandle>& indices) {
    eraseByIndices(edges, indices.begin(), indices.end());
}

void LineGraph::removeVerticesByIndex(std::unordered_set<VertexHandle>& indices) {
    AdjacencyList adjacency = computeAdjacency();

    std::size_t offset = 0;
    const std::size_t old_vertices = numVertices();

    for (VertexHandle vh = 0; vh < old_vertices; vh++) {

        if (indices.find(vh) != indices.end()) {
            // remove vertex and adjust offset
            points.erase(points.begin() + vh - offset);
            offset++;
            continue;
        }

        if (offset != 0) {
            for (EdgeHandle eh : adjacency.get(vh)) {
                // update to new VertexHandle
                if (edges[eh].first == vh) {
                    edges[eh].first -= offset;
                } else {
                    edges[eh].second -= offset;
                }
            }
        }
    }
}

const vec2_t& LineGraph::getPoint(VertexHandle v) const {
    return points[v];
}

vec2_t& LineGraph::getPoint(VertexHandle v) {
    return points[v];
}

const LineGraph::Edge& LineGraph::getEdge(EdgeHandle e) const {
    return edges[e];
}

LineGraph::Edge& LineGraph::getEdge(EdgeHandle e) {
    return edges[e];
}

const std::vector<vec2_t>& LineGraph::getPoints() const {
    return points;
}

const std::vector<LineGraph::Edge>& LineGraph::getEdges() const {
    return edges;
}

AxisAlignedBoundingBox LineGraph::computeBoundingBox() const {
    if (points.size() == 0) {
        throw std::runtime_error("empty polygon has no bounding box");
    }

    AxisAlignedBoundingBox aabb;
    aabb.min = points[0];
    aabb.max = points[0];

    for (const vec2_t& p : points) {
        if (p[0] < aabb.min[0]) {
            aabb.min[0] = p[0];
        }
        if (p[1] < aabb.min[1]) {
            aabb.min[1] = p[1];
        }
        if (p[0] > aabb.max[0]) {
            aabb.max[0] = p[0];
        }
        if (p[1] > aabb.max[1]) {
            aabb.max[1] = p[1];
        }
    }

    return aabb;
}

LineGraph::AdjacencyList LineGraph::computeAdjacency() const {
    return AdjacencyList(*this);
}

void LineGraph::removeDegeneratedGeometry() {
    ScopeTimer timer("Remove degenerated geomeetry");

    AdjacencyList adjacency = computeAdjacency();

    std::unordered_set<VertexHandle> deleted_vertices;
    std::vector<EdgeHandle> deleted_edges;

    for (EdgeHandle eh = 0; eh < edges.size(); eh++) {
        const Edge& e = getEdge(eh);

        const vec2_t& p0 = getPoint(e.first);
        const vec2_t& p1 = getPoint(e.second);

        // remove degenerated edge
        if (p0[0] == p1[0] && p0[1] == p1[1]) {

            // mark duplicate vertex and edge as deleted
            deleted_vertices.insert(e.second);
            deleted_edges.push_back(eh);

            const EdgeHandle next = adjacency.getNext(eh);

            // connect following edge to remaining point
            if (edges[next].first == e.second) {
                edges[next].first = e.first;
            } else {
                edges[next].second = e.first;
            }

            // fix adjacency
            if (adjacency.edges[e.first].front() == eh) {
                adjacency.edges[e.first].front() = next;
            } else {
                adjacency.edges[e.first].back() = next;
            }
        }
    }

    // change VertexHandles in edges
    std::size_t offset = 0;
    const std::size_t old_vertices = numVertices();

    for (VertexHandle vh = 0; vh < old_vertices; vh++) {

        if (deleted_vertices.find(vh) != deleted_vertices.end()) {
            // remove vertex and adjust offset
            points.erase(points.begin() + vh - offset);
            offset++;
            continue;
        }

        if (offset != 0) {
            for (EdgeHandle eh : adjacency.get(vh)) {
                // update to new VertexHandle
                if (edges[eh].first == vh) {
                    edges[eh].first -= offset;
                } else {
                    edges[eh].second -= offset;
                }
            }
        }
    }

    // remove edges
    eraseByIndices(edges, deleted_edges.begin(), deleted_edges.end());
}

bool LineGraph::hasSelfIntersection() const {  // TODO: improve performance
    for (const Edge& e1 : getEdges()) {
        const LineSegment l1 = {getPoint(e1.first), getPoint(e1.second)};

        for (const Edge& e2 : getEdges()) {

            const bool shared_corner = e1.first == e2.first || e1.first == e2.second ||
                                       e1.second == e2.first || e1.second == e2.second;

            if (shared_corner) {
                continue;
            }

            const LineSegment l2 = {getPoint(e2.first), getPoint(e2.second)};
            if (lineIntersection(l1, l2)) {
                return true;
            }
        }
    }
    return false;
}


LineGraphAdjacencyList::LineGraphAdjacencyList(const LineGraph& lg)
    : lg(lg) {

    edges.resize(lg.numVertices());

    for (EdgeHandle eh = 0; eh < lg.numEdges(); eh++) {

        edges[lg.getEdge(eh).first].push_back(eh);
        edges[lg.getEdge(eh).second].push_back(eh);
    }
}

std::list<LineGraph::VertexHandle> LineGraphAdjacencyList::getNeighbors(VertexHandle vh) const {
    std::list<LineGraph::VertexHandle> neighbors;

    for (EdgeHandle eh : edges[vh]) {
        const LineGraph::Edge& e = lg.getEdge(eh);

        if (e.first != vh) {
            neighbors.push_back(e.first);
        }
        if (e.second != vh) {
            neighbors.push_back(e.second);
        }
    }
    return neighbors;
}

LineGraphAdjacencyList::EdgeHandle LineGraphAdjacencyList::getPrev(EdgeHandle eh) const {
    const std::list<EdgeHandle>& adj = edges[lg.getEdge(eh).first];

    const EdgeHandle e = adj.front();
    if (e == eh) {
        return adj.back();
    }
    return e;
}

LineGraphAdjacencyList::EdgeHandle LineGraphAdjacencyList::getNext(EdgeHandle eh) const {
    const std::list<EdgeHandle>& adj = edges[lg.getEdge(eh).second];

    const EdgeHandle e = adj.front();
    if (e == eh) {
        return adj.back();
    }
    return e;
}

}
