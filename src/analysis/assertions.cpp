
#include "assertions.h"

#include <vector>

#include <geometry/line_intersection.h>
#include <util.h>

namespace omg {
namespace analysis {

bool isClosed(const LineGraph& lg) {
    const LineGraph::AdjacencyList adj = lg.computeAdjacency();

    for (auto ls : adj.edges) {

        if (ls.size() != 2) {
            return false;
        }
    }
    return true;
}

bool hasValenceGreaterTwo(const LineGraph& lg) {

    const LineGraph::AdjacencyList adj = lg.computeAdjacency();

    for (auto ls : adj.edges) {

        if (ls.size() > 2) {
            return true;
        }
    }
    return false;
}

bool hasUnusedVertices(const LineGraph& lg) {
    std::vector<bool> used(lg.numVertices());

    for (const LineGraph::Edge& e : lg.getEdges()) {

        // mark all vertices used in an edge
        used[e.first] = true;
        used[e.second] = true;
    }

    for (bool b : used) {
        if (!b) {
            return true;
        }
    }
    return false;
}

bool hasOneVertexLoops(const LineGraph& lg) {
    for (const LineGraph::Edge& e : lg.getEdges()) {

        if (e.first == e.second) {
            return true;
        }
    }
    return false;
}

std::size_t countTwoVertexLoops(const LineGraph& lg) {
    using EHandle = LineGraph::EdgeHandle;
    using VHandle = LineGraph::VertexHandle;

    std::size_t cnt = 0;

    const LineGraph::AdjacencyList adj = lg.computeAdjacency();

    for (VHandle vh = 0; vh < lg.numVertices(); vh++) {

        if (adj.get(vh).size() != 2) {
            continue;
        }

        const EHandle e0 = adj.get(vh).front();
        const EHandle e1 = adj.get(vh).back();

        const VHandle v0 = lg.getEdge(e0).first;
        const VHandle v1 = lg.getEdge(e0).second;
        const VHandle v2 = lg.getEdge(e1).first;
        const VHandle v3 = lg.getEdge(e1).second;

        // find other two vertices
        const VHandle a = v0 == vh ? v1 : v0;
        const VHandle b = v2 == vh ? v3 : v2;

        if (a == b) {
            cnt++;
        }
    }

    return cnt;
}

std::size_t countZeroLengthEdges(const LineGraph& lg) {
    std::size_t cnt = 0;

    for (const LineGraph::Edge& e : lg.getEdges()) {

        const vec2_t& p0 = lg.getPoint(e.first);
        const vec2_t& p1 = lg.getPoint(e.second);

        // start and end point have exactly the same coordinates
        if (p0[0] == p1[0] && p0[1] == p1[1]) {
            cnt++;
        }
    }

    return cnt;
}

std::size_t countIdenticalPoints(const LineGraph& lg) {
    std::size_t cnt = 0;

    // key in map is x coord, sets store y coords
    std::unordered_map<real_t, std::unordered_set<real_t>> points;

    for (auto p : lg.getPoints()) {

        auto it = points.find(p[0]);

        if (it == points.end()) {
            points[p[0]] = {p[1]};  // new x coord

        } else if (it->second.find(p[1]) == it->second.end()) {
            it->second.insert(p[1]);  // new y coord

        } else {
            cnt++;  // found same x and y coord
        }
    }

    return cnt;
}

std::size_t countCollinearEdges(const LineGraph& lg) {
    using EHandle = LineGraph::EdgeHandle;
    using VHandle = LineGraph::VertexHandle;

    std::size_t cnt = 0;

    const LineGraph::AdjacencyList adj = lg.computeAdjacency();

    for (auto ls : adj.edges) {

        if (ls.size() != 2) {
            continue;
        }

        const EHandle e0 = ls.front();
        const EHandle e1 = ls.back();

        const VHandle v0 = lg.getEdge(e0).first;
        const VHandle v1 = lg.getEdge(e0).second;
        const VHandle v2 = lg.getEdge(e1).first;
        const VHandle v3 = lg.getEdge(e1).second;

        if (collinear({lg.getPoint(v0), lg.getPoint(v1)}, {lg.getPoint(v2), lg.getPoint(v3)})) {
            cnt++;
        }
    }

    return cnt;
}

std::size_t countAngles(const LineGraph& lg, real_t degrees) {
    using EHandle = LineGraph::EdgeHandle;
    using VHandle = LineGraph::VertexHandle;

    const real_t cos_angle = std::cos(toRadians(degrees));

    std::size_t cnt = 0;

    const LineGraph::AdjacencyList adj = lg.computeAdjacency();

    for (VHandle vh = 0; vh < lg.numVertices(); vh++) {

        if (adj.get(vh).size() != 2) {
            continue;
        }

        const EHandle e0 = adj.get(vh).front();
        const EHandle e1 = adj.get(vh).back();

        const VHandle v0 = lg.getEdge(e0).first;
        const VHandle v1 = lg.getEdge(e0).second;
        const VHandle v2 = lg.getEdge(e1).first;
        const VHandle v3 = lg.getEdge(e1).second;

        // find correct vertex order
        const VHandle a = v0 == vh ? v1 : v0;
        const VHandle b = vh;
        const VHandle c = v2 == vh ? v3 : v2;

        vec2_t ba = lg.getPoint(a) - lg.getPoint(b);
        vec2_t bc = lg.getPoint(c) - lg.getPoint(b);

        if (ba.norm() == 0 || bc.norm() == 0) {
            continue;
        }

        ba.normalize();
        bc.normalize();

        if (ba.dot(bc) == cos_angle) {
            cnt++;
        }
    }

    return cnt;
}

}
}
