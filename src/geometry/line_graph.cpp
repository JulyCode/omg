
#include "line_graph.h"

namespace omg {

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

}
