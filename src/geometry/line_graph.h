#pragma once

#include <vector>

#include <geometry/types.h>

namespace omg {

class LineGraph {
public:
    using VertexHandle = std::size_t;
    using EdgeHandle = std::size_t;
    using Edge = std::pair<VertexHandle, VertexHandle>;  // indices of incident vertices

public:
    LineGraph() = default;

    static LineGraph createRectangle(const AxisAlignedBoundingBox& aabb);

    VertexHandle addVertex(const vec2_t& p);
    EdgeHandle addEdge(VertexHandle v1, VertexHandle v2);

    const vec2_t& getPoint(VertexHandle v) const;
    const Edge& getEdge(EdgeHandle e) const;

    inline std::size_t numVertices() const { return points.size(); }
    inline std::size_t numEdges() const { return edges.size(); }

    const std::vector<vec2_t>& getPoints() const;
    const std::vector<Edge>& getEdges() const;

    AxisAlignedBoundingBox computeBoundingBox() const;

    bool hasSelfIntersection() const;

private:
    std::vector<vec2_t> points;
    std::vector<Edge> edges;
};

}
