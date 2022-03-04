
#include "simplification.h"

#include <util.h>
#include <geometry/line_intersection.h>
#include <geometry/line_graph.h>

namespace omg {

static std::size_t collapse(HEPolygon& poly, const SizeFunction& size, real_t min_angle_deg) {

    const real_t min_angle_cos = std::cos(toRadians(min_angle_deg));

    std::size_t count = 0;
    for (HEPolygon::HalfEdgeHandle heh : poly.halfEdges()) {

        if (poly.isDegenerated()) {
            break;
        }

        const vec2_t& p1 = poly.startPoint(heh);
        const vec2_t& p2 = poly.endPoint(heh);
        const vec2_t& p3 = poly.endPoint(poly.nextHalfEdge(heh));

        const real_t target = size.getValue((p1 + p2) / 2);

        bool remove = (p2 - p1).norm() < target;

        // fix degenerated areas
        remove |= degreesToMeters((p3 - p1).norm()) < 1;  // TODO: use geoDistance?

        // remove small angles
        remove |= (p1 - p2).normalized().dot((p3 - p2).normalized()) > min_angle_cos;

        if (remove) {
            poly.collapse(heh, 0);
            count++;
        }
    }
    return count;
}

void simplifyPolygon(HEPolygon& poly, const SizeFunction& size, real_t min_angle_deg) {
    while (collapse(poly, size, min_angle_deg) != 0) {}
}


using EdgeHandle = LineGraph::EdgeHandle;
using VertexHandle = LineGraph::VertexHandle;

struct RemovalData {
    LineGraph& graph;

    LineGraph::AdjacencyList adjacency;

    std::unordered_set<VertexHandle> deleted_vertices;
    std::vector<EdgeHandle> deleted_edges;
};

static VertexHandle removeZeroLengthEdge(EdgeHandle eh, RemovalData& data) {
    const LineGraph::Edge& e = data.graph.getEdge(eh);

    // mark duplicate vertex and edge as deleted
    data.deleted_vertices.insert(e.second);
    data.deleted_edges.push_back(eh);

    const EdgeHandle next = data.adjacency.getNext(eh);

    if (next == eh) {  // deleted a boundary vertex
        // fix adjacency
        if (data.adjacency.edges[e.first].front() == eh) {
            data.adjacency.edges[e.first].erase(data.adjacency.edges[e.first].begin());
        } else {
            data.adjacency.edges[e.first].erase(std::next(data.adjacency.edges[e.first].begin()));
        }

    } else {
        // connect following edge to remaining point
        LineGraph::Edge& next_edge = data.graph.getEdge(next);
        if (next_edge.first == e.second) {
            next_edge.first = e.first;
        } else {
            next_edge.second = e.first;
        }

        // fix adjacency
        if (data.adjacency.edges[e.first].front() == eh) {
            data.adjacency.edges[e.first].front() = next;
        } else {
            data.adjacency.edges[e.first].back() = next;
        }
    }

    return e.second;  // deleted vertex
}

void removeDegeneratedGeometry(LineGraph& graph) {

    ScopeTimer timer("Remove degenerated geomeetry");
    int cnt1 = 0;
    int cnt2 = 0;

    RemovalData data = {graph, graph.computeAdjacency(), {}, {}};

    std::unordered_set<EdgeHandle> todo;
    for (EdgeHandle eh = 0; eh < graph.numEdges(); eh++) {

        const LineGraph::Edge& e = graph.getEdge(eh);
        const vec2_t& p0 = graph.getPoint(e.first);
        const vec2_t& p1 = graph.getPoint(e.second);

        if (p0[0] == p1[0] && p0[1] == p1[1]) {
            todo.insert(eh);
        }
    }

    // compute points with identical coordinates
    // first key is x, second is y
    std::unordered_map<real_t, std::unordered_map<real_t, VertexHandle>> identical_points;

    for (VertexHandle vh = 0; vh < graph.numVertices(); vh++) {
        const vec2_t& p = graph.getPoint(vh);

        auto it_x = identical_points.find(p[0]);

        if (it_x == identical_points.end()) {
            identical_points[p[0]] = {{p[1], vh}};  // new x coord

        } else {
            auto it_y = it_x->second.find(p[1]);

            if (it_y == it_x->second.end()) {
                it_x->second.insert({p[1], vh});  // new y coord

            } else {
                // identical point found
                VertexHandle other = it_y->second;
                cnt1++;

                // if points are directly connected
                for (VertexHandle n : data.adjacency.getNeighbors(vh)) {
                    if (n == other) {
                        // get connecting edge
                        EdgeHandle eh = data.adjacency.get(vh).front();
                        if (graph.getEdge(eh).first != n && graph.getEdge(eh).second != n) {
                            eh = data.adjacency.get(vh).back();
                        }
                        if (todo.find(eh) == todo.end()) {
                            std::cout << "what" << std::endl;
                        }
                        todo.erase(eh);

                        VertexHandle del = removeZeroLengthEdge(eh, data);

                        if (del == other) {
                            it_y->second = vh;  // switch stored VertexHandle if it was deleted
                        }

                        auto& new_adj = data.adjacency.get(it_y->second);
                        if (new_adj.front() == new_adj.back()) {
                            // remove self connected vertex
                            data.deleted_vertices.insert(it_y->second);
                            data.deleted_edges.push_back(new_adj.front());

                            if (todo.find(new_adj.front()) == todo.end()) {
                                std::cout << "what" << std::endl;
                            }
                            todo.erase(new_adj.front());

                            it_x->second.erase(p[1]);
                            cnt2++;
                        }
                        cnt2++;
                        break;
                    }
                }
            }
        }
    }

    // remove edges and vertices
    graph.removeVerticesByIndex(data.deleted_vertices);
    graph.removeEdgesByIndex(data.deleted_edges);
}

}
