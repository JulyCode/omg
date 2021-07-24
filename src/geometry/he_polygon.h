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
    explicit HEPolygon(const std::vector<vec2_t>& polygon);

    // split the halfedge at location from 0 to 1 and return the new vertex in the middle
    VertexHandle split(HalfEdgeHandle heh, real_t location = 0.5);

    // collapse the halfedge and return the remaining point
    VertexHandle collapse(HalfEdgeHandle heh, real_t location = 0.5);

    const vec2_t& point(VertexHandle vh) const;
    vec2_t& point(VertexHandle vh);

    VertexRange vertices() const;
    HalfEdgeRange halfEdges() const;

    VertexIterator verticesBegin() const;
    VertexIterator verticesEnd() const;

    HalfEdgeIterator halfEdgesBegin() const;
    HalfEdgeIterator halfEdgesEnd() const;

    inline std::size_t numVertices() const { return points.size() - deleted.size(); }
    inline std::size_t numHalfEdges() const { return numVertices(); }

    // this is *not* the number of valid vertices
    inline std::size_t size() const { return points.size(); }

    bool isValid(std::size_t handle) const;

    // invalidates all handles!
    void garbageCollect();

    LineGraph toLineGraph() const;

private:
    std::vector<vec2_t> points;
    std::vector<HalfEdge> half_edges;

    std::unordered_set<std::size_t> deleted;
};


template<typename Handle>
class HEPolygonIterator {
public:
    HEPolygonIterator(const HEPolygon& poly, Handle start);

    HEPolygonIterator& operator++();
    HEPolygonIterator operator++(int);

    Handle operator*() const;
    Handle operator->() const;

    bool operator==(const HEPolygonIterator& other) const;
    bool operator!=(const HEPolygonIterator& other) const;

private:
    Handle handle;
    const HEPolygon& poly;
};


template<typename Iterator>
class HEPolygonRange {
public:
    explicit HEPolygonRange(const HEPolygon& poly) : poly(poly) {}

    inline Iterator begin() const { return Iterator(poly, 0); }
    inline Iterator end() const { return Iterator(poly, poly.size()); }

private:
    const HEPolygon& poly;
};

}
