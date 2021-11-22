#pragma once

#include <vector>
#include <list>

#include <geometry/he_polygon.h>

namespace omg {

class LineGraphAdjacencyList;

class LineGraph {
public:
    using VertexHandle = std::size_t;
    using EdgeHandle = std::size_t;
    using Edge = std::pair<VertexHandle, VertexHandle>;  // indices of incident vertices

    using AdjacencyList = LineGraphAdjacencyList;

public:
    LineGraph() = default;
    explicit LineGraph(const HEPolygon& poly);

    static LineGraph createRectangle(const AxisAlignedBoundingBox& aabb);
    static LineGraph combinePolygons(const std::vector<HEPolygon>& polys);

    VertexHandle addVertex(const vec2_t& p);
    EdgeHandle addEdge(VertexHandle v1, VertexHandle v2);

    void removeEdgesByIndex(std::vector<EdgeHandle>& indices);
    void removeVerticesByIndex(std::unordered_set<VertexHandle>& indices);

    const vec2_t& getPoint(VertexHandle v) const;
    vec2_t& getPoint(VertexHandle v);

    const Edge& getEdge(EdgeHandle e) const;
    Edge& getEdge(EdgeHandle e);

    inline std::size_t numVertices() const { return points.size(); }
    inline std::size_t numEdges() const { return edges.size(); }

    const std::vector<vec2_t>& getPoints() const;
    const std::vector<Edge>& getEdges() const;

    AxisAlignedBoundingBox computeBoundingBox() const;

    AdjacencyList computeAdjacency() const;

    // invalidates all handles!
    void removeDegeneratedGeometry();

    bool hasSelfIntersection() const;

private:
    std::vector<vec2_t> points;
    std::vector<Edge> edges;
};


class LineGraphAdjacencyList {
public:
    using EdgeHandle = LineGraph::EdgeHandle;
    using VertexHandle = LineGraph::VertexHandle;

    explicit LineGraphAdjacencyList(const LineGraph& lg);

    const std::list<EdgeHandle>& get(VertexHandle vh) const { return edges[vh]; }
    std::list<EdgeHandle>& get(VertexHandle vh) { return edges[vh]; }

    std::list<VertexHandle> getNeighbors(VertexHandle vh) const;

    EdgeHandle getPrev(EdgeHandle eh) const;
    EdgeHandle getNext(EdgeHandle eh) const;

    std::vector<std::list<EdgeHandle>> edges;  // incident edges per vertex

private:
    const LineGraph& lg;
};

}
