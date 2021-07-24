
#include "he_polygon.h"

#include <cstring>

namespace omg {

HEPolygon::HEPolygon(const std::vector<vec2_t>& polygon) {

    // copy points
    points.insert(points.begin(), polygon.begin(), polygon.end());

    for (std::size_t i = 0; i < points.size(); i++) {

        // create halfedge for vertex i
        const HalfEdgeHandle prev = (i == 0) ? points.size() - 1 : i - 1;
        const HalfEdgeHandle next = (i + 1) % points.size();
        half_edges.push_back({prev, next});
    }
}


HEPolygon::VertexHandle HEPolygon::split(HalfEdgeHandle heh, real_t location) {
    const VertexHandle start = heh;
    const VertexHandle end = half_edges[heh].next;
    const VertexHandle mid = points.size();

    // add new point
    const vec2_t point = points[start] + (points[end] - points[start]) * location;
    points.push_back(point);

    // add new halfedge
    half_edges.push_back({start, end});

    // modify halfedges
    half_edges[start].next = mid;
    half_edges[end].prev = mid;

    return mid;
}

HEPolygon::VertexHandle HEPolygon::collapse(HalfEdgeHandle heh, real_t location) {
    const VertexHandle start = heh;
    const VertexHandle end = half_edges[heh].next;
    const VertexHandle end_next = half_edges[end].next;

    // move point to middle
    points[start] = points[start] + (points[end] - points[start]) * location;

    // modify halfedges
    half_edges[start].next = end_next;
    half_edges[end_next].prev = start;

    // mark as deleted
    deleted.insert(heh);

    return start;
}


const vec2_t& HEPolygon::point(VertexHandle vh) const {
    return points[vh];
}

vec2_t& HEPolygon::point(VertexHandle vh) {
    return points[vh];
}


HEPolygon::VertexRange HEPolygon::vertices() const { return VertexRange(*this); }
HEPolygon::HalfEdgeRange HEPolygon::halfEdges() const { return HalfEdgeRange(*this); }

HEPolygon::VertexIterator HEPolygon::verticesBegin() const { return VertexIterator(*this, 0); }
HEPolygon::VertexIterator HEPolygon::verticesEnd() const { return VertexIterator(*this, size()); }

HEPolygon::HalfEdgeIterator HEPolygon::halfEdgesBegin() const { return HalfEdgeIterator(*this, 0); }
HEPolygon::HalfEdgeIterator HEPolygon::halfEdgesEnd() const { return HalfEdgeIterator(*this, size()); }


bool HEPolygon::isValid(std::size_t handle) const {
    return handle < points.size() && deleted.find(handle) == deleted.end();
}


void HEPolygon::garbageCollect() {
    // copy to vector
    std::vector<std::size_t> del;
    del.insert(del.begin(), deleted.begin(), deleted.end());

    std::sort(del.begin(), del.end());
    del.push_back(points.size());  // copy until end

    vec2_t* points_begin = points.data();
    HalfEdge* he_begin = half_edges.data();

    // move parts between deleted elements to the left
    for (std::size_t i = 0; i < del.size() - 1; i++) {

        const std::size_t dst = del[i] - i;
        const std::size_t src = del[i] + 1;
        const std::size_t count = del[i + 1] - del[i] - 1;

        if (count == 0) {
            continue;
        }

        std::memmove(points_begin + dst, points_begin + src, count * sizeof(vec2_t));
        std::memmove(he_begin + dst, he_begin + src, count * sizeof(HalfEdge));
    }

    // delete ramaining elements at the end
    points.erase(points.end() - deleted.size(), points.end());
    half_edges.erase(half_edges.end() - deleted.size(), half_edges.end());

    deleted.clear();
}

LineGraph HEPolygon::toLineGraph() const {
    LineGraph graph;

    for (std::size_t i = 0; i < points.size(); i++) {
        if (isValid(i)) {

            graph.addVertex(points[i]);

            graph.addEdge(i, half_edges[i].next);
        }
    }

    return graph;
}


template<typename Handle>
HEPolygonIterator<Handle>::HEPolygonIterator(const HEPolygon& poly, Handle start)
    : handle(start), poly(poly) {}

template<typename Handle>
HEPolygonIterator<Handle>& HEPolygonIterator<Handle>::operator++() {
    do {
        handle++;
    } while (handle < poly.size() && !poly.isValid(handle));

    return *this;
}

template<typename Handle>
HEPolygonIterator<Handle> HEPolygonIterator<Handle>::operator++(int) {
    HEPolygonIterator<Handle> it = *this;
    operator++();
    return it;
}

template<typename Handle>
Handle HEPolygonIterator<Handle>::operator*() const {
    return handle;
}

template<typename Handle>
Handle HEPolygonIterator<Handle>::operator->() const {
    return handle;
}

template<typename Handle>
bool HEPolygonIterator<Handle>::operator==(const HEPolygonIterator& other) const {
    return handle == other.handle && &poly == &other.poly;
}

template<typename Handle>
bool HEPolygonIterator<Handle>::operator!=(const HEPolygonIterator& other) const {
    return !operator==(other);
}

template class HEPolygonIterator<std::size_t>;

}
