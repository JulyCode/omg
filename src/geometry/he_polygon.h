#pragma once

#include <unordered_set>

#include <geometry/line_graph.h>

namespace omg {

template<typename Handle>
class HEPolygonIterator;

template<typename Iterator>
class HEPolygonRange;


class HEPolygon {
public:
    using VertexHandle = std::size_t;
    using HalfEdgeHandle = std::size_t;

    struct HalfEdge {
        HalfEdgeHandle prev, next;
    };

    using VertexIterator = HEPolygonIterator<VertexHandle>;
    using HalfEdgeIterator = HEPolygonIterator<HalfEdgeHandle>;

    using VertexRange = HEPolygonRange<VertexIterator>;
    using HalfEdgeRange = HEPolygonRange<HalfEdgeIterator>;

public:
    HEPolygon() = default;
    explicit HEPolygon(const std::vector<vec2_t>& polygon);

    void setPoints(const std::vector<vec2_t>& polygon);

    // split the halfedge at location from 0 to 1 and return the new vertex in the middle
    VertexHandle split(HalfEdgeHandle heh, real_t location = 0.5);

    // collapse the halfedge and return the remaining point
    VertexHandle collapse(HalfEdgeHandle heh, real_t location = 0.5);

    inline const vec2_t& point(VertexHandle vh) const { return points[vh]; }
    inline vec2_t& point(VertexHandle vh) { return points[vh]; }

    inline HalfEdgeHandle nextHalfEdge(HalfEdgeHandle heh) const { return half_edges[heh].next; }
    inline HalfEdgeHandle prevHalfEdge(HalfEdgeHandle heh) const { return half_edges[heh].prev; }

    inline VertexHandle nextVertex(VertexHandle vh) const { return half_edges[vh].next; }
    inline VertexHandle prevVertex(VertexHandle vh) const { return half_edges[vh].prev; }

    inline VertexHandle startVertex(HalfEdgeHandle heh) const { return heh; }
    inline VertexHandle endVertex(HalfEdgeHandle heh) const { return half_edges[heh].next; }

    inline const vec2_t& startPoint(HalfEdgeHandle heh) const { return points[heh]; }
    inline const vec2_t& endPoint(HalfEdgeHandle heh) const { return points[half_edges[heh].next]; }
    inline vec2_t& startPoint(HalfEdgeHandle heh) { return points[heh]; }
    inline vec2_t& endPoint(HalfEdgeHandle heh) { return points[half_edges[heh].next]; }

    inline HalfEdgeHandle outgoingHalfEdge(VertexHandle vh) const { return vh; }
    inline HalfEdgeHandle incomingHalfEdge(VertexHandle vh) const { return half_edges[vh].prev; }

    // unordered iterators
    VertexRange vertices(VertexHandle start = 0) const;
    HalfEdgeRange halfEdges(VertexHandle start = 0) const;

    // ordered iterators
    VertexRange verticesOrdered(VertexHandle start = 0) const;
    HalfEdgeRange halfEdgesOrdered(HalfEdgeHandle start = 0) const;

    inline std::size_t numVertices() const { return points.size() - deleted.size(); }
    inline std::size_t numHalfEdges() const { return numVertices(); }

    bool isValid(std::size_t handle) const;

    bool isDegenerated() const;

    // invalidates all handles!
    void garbageCollect();

    AxisAlignedBoundingBox computeBoundingBox() const;
    real_t computeArea() const;

    bool hasSelfIntersection() const;

    bool pointInPolygon(const vec2_t& p, const vec2_t& dir = {1, 1}) const;

    LineGraph toLineGraph() const;

private:
    std::vector<vec2_t> points;
    std::vector<HalfEdge> half_edges;

    std::unordered_set<std::size_t> deleted;

    template<typename Handle>
    friend class HEPolygonIterator;

    template<typename Iterator>
    friend class HEPolygonRange;
};


template<typename Handle>
class HEPolygonIterator {
public:
    HEPolygonIterator(const HEPolygon& poly, Handle start, bool ordered);

    HEPolygonIterator& operator++();
    HEPolygonIterator operator++(int);

    HEPolygonIterator& operator+(std::size_t n);

    Handle operator*() const;

    bool operator==(const HEPolygonIterator& other) const;
    bool operator!=(const HEPolygonIterator& other) const;

    void toEnd();

private:
    const HEPolygon& poly;

    Handle start;
    Handle handle;

    const bool ordered;
    bool is_end;
};


template<typename Iterator>
class HEPolygonRange {
public:
    explicit HEPolygonRange(const Iterator& first) : first(first), last(first) {
        last.toEnd();
    }

    inline Iterator begin() const { return first; }
    inline const Iterator& end() const { return last; }

private:
    const Iterator first;
    Iterator last;
};

}
