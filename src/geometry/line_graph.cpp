
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

const vec2_t& LineGraph::getPoint(VertexHandle v) const {
    return points[v];
}

const LineGraph::Edge& LineGraph::getEdge(EdgeHandle e) const {
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
