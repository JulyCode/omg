
#include "he_polygon.h"

#include <iostream>
#include <random>

#include <geometry/line_intersection.h>

namespace omg {

HEPolygon::HEPolygon(const std::vector<vec2_t>& polygon) {
    setPoints(polygon);
}

void HEPolygon::setPoints(const std::vector<vec2_t>& polygon) {
    if (polygon.size() < 3) {
        throw std::runtime_error("polygon is degenerated");
    }
    points.clear();
    half_edges.clear();

    // compute polygon orientation
    // see https://en.wikipedia.org/wiki/Curve_orientation

    // get point on convex hull
    std::size_t index = 0;
    vec2_t pos = polygon[0];

    for (std::size_t i = 0; i < polygon.size(); i++) {
        const vec2_t& p = polygon[i];

        if (p[0] < pos[0] || (p[0] == pos[0] && p[1] < pos[1])) {
            pos = p;
            index = i;
        }
    }

    // get neighbor points
    const vec2_t& prev = polygon[(index - 1) % polygon.size()];
    const vec2_t& next = polygon[(index + 1) % polygon.size()];

    // compute determinant
    const real_t det = (pos[0] - prev[0]) * (next[1] - prev[1]) - (next[0] - prev[0]) * (pos[1] - prev[1]);

    // copy points in counter-clockwise order
    if (det > 0) {
        points = polygon;
    } else {
        points.resize(polygon.size());
        std::reverse_copy(polygon.begin(), polygon.end(), points.begin());
    }

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
    const VertexHandle prev = half_edges[start].prev;

    // move point to middle
    points[end] = points[start] + (points[end] - points[start]) * location;

    // modify halfedges
    half_edges[end].prev = prev;
    half_edges[prev].next = end;

    // mark as deleted
    deleted.insert(heh);

    return end;
}


HEPolygon::VertexRange HEPolygon::vertices(VertexHandle start) const {
    return VertexRange(VertexIterator(*this, start, false));
}

HEPolygon::HalfEdgeRange HEPolygon::halfEdges(VertexHandle start) const {
    return HalfEdgeRange(HalfEdgeIterator(*this, start, false));
}

HEPolygon::VertexRange HEPolygon::verticesOrdered(VertexHandle start) const {
    return VertexRange(VertexIterator(*this, start, true));
}

HEPolygon::HalfEdgeRange HEPolygon::halfEdgesOrdered(HalfEdgeHandle start) const {
    return HalfEdgeRange(HalfEdgeIterator(*this, start, true));
}


bool HEPolygon::isValid(std::size_t handle) const {
    return handle < points.size() && deleted.find(handle) == deleted.end();
}

bool HEPolygon::isDegenerated() const {
    return numHalfEdges() < 3;
}


void HEPolygon::garbageCollect() {
    // copy to vector
    std::vector<std::size_t> del;
    del.insert(del.begin(), deleted.begin(), deleted.end());

    std::sort(del.begin(), del.end());
    del.push_back(points.size());  // copy until end

    // move parts between deleted elements to the left
    for (std::size_t i = 0; i < del.size() - 1; i++) {

        const std::size_t dst = del[i] - i;
        const std::size_t src = del[i] + 1;
        const std::size_t count = del[i + 1] - del[i] - 1;

        if (count == 0) {
            continue;
        }

        for (std::size_t j = 0; j < count; j++) {

            // copy point and halfedge
            points[dst + j] = points[src + j];
            half_edges[dst + j] = half_edges[src + j];

            // adjust connectivity
            half_edges[half_edges[dst + j].next].prev = dst + j;
            half_edges[half_edges[dst + j].prev].next = dst + j;
        }
    }

    // delete ramaining elements at the end
    points.erase(points.end() - deleted.size(), points.end());
    half_edges.erase(half_edges.end() - deleted.size(), half_edges.end());

    deleted.clear();
}

bool HEPolygon::hasGarbage() const {
    return !deleted.empty();
}

AxisAlignedBoundingBox HEPolygon::computeBoundingBox() const {
    if (points.size() == 0) {
        throw std::runtime_error("empty polygon has no bounding box");
    }

    AxisAlignedBoundingBox aabb;
    bool init = true;

    for (VertexHandle v : vertices()) {
        const vec2_t& p = point(v);

        if (p[0] < aabb.min[0] || init) {
            aabb.min[0] = p[0];
        }
        if (p[1] < aabb.min[1] || init) {
            aabb.min[1] = p[1];
        }
        if (p[0] > aabb.max[0] || init) {
            aabb.max[0] = p[0];
        }
        if (p[1] > aabb.max[1] || init) {
            aabb.max[1] = p[1];
        }

        init = false;
    }

    return aabb;
}

real_t HEPolygon::computeArea() const {
    if (points.size() == 0) {
        throw std::runtime_error("empty polygon has no area");
    }

    // shoelace algorithm
    // see http://apodeline.free.fr/FAQ/CGAFAQ/CGAFAQ-3.html
    real_t area = 0;
    for (VertexHandle v : vertices()) {

        const vec2_t& p1 = point(v);
        const vec2_t& p2 = point(nextVertex(v));
        area += (p1[0] + p2[0]) * (p2[1] - p1[1]);
    }

    return area / 2;
}

bool HEPolygon::hasSelfIntersection() const {  // very slow
    for (HalfEdgeHandle e1 : halfEdges()) {
        const LineSegment l1 = {startPoint(e1), endPoint(e1)};

        for (HalfEdgeHandle e2 : halfEdges(e1)) {

            const bool shared_corner = nextHalfEdge(e1) == e2 || prevHalfEdge(e1) == e2;

            if (shared_corner) {
                continue;
            }

            const LineSegment l2 = {startPoint(e2), endPoint(e2)};
            if (lineIntersection(l1, l2)) {
                return true;
            }
        }
    }
    return false;
}

PointInPolygon HEPolygon::pointInPolygon(const vec2_t& p, vec2_t dir) const {
    // test with bounding box
    const AxisAlignedBoundingBox aabb = computeBoundingBox();
    if (p[0] < aabb.min[0] || p[0] > aabb.max[0] || p[1] < aabb.min[1] || p[1] > aabb.max[1]) {
        return OUTSIDE;
    }

    std::size_t intersections;

    int tries = 0;
    bool repeat;
    do {
        repeat = false;

        // construct ray
        const real_t max_length = std::max(aabb.max[0] - aabb.min[0], aabb.max[1] - aabb.min[1]);
        const vec2_t end = p + 2 * max_length * dir.normalized();
        const LineSegment ray = {p, end};

        intersections = 0;
        for (HalfEdgeHandle heh : halfEdges()) {

            const LineSegment edge = {startPoint(heh), endPoint(heh)};

            std::optional<real_t> u = lineIntersectionFactor(ray, edge);
            if (u) {

                // special cases
                if (*u == 0) {
                    return ON_EDGE;
                }

                std::optional<real_t> t = lineIntersectionFactor(edge, ray);

                if (*t == 0 || *t == 1 || collinear(edge, ray)) {

                    // generate new random direction
                    static std::default_random_engine rnd;
                    static std::uniform_real_distribution<real_t> dis(-1, 1);
                    dir[0] = dis(rnd);
                    dir[1] = dis(rnd);
                    repeat = true;
                    break;
                }
                intersections++;
            }
        }

        ++tries;
        if (tries > 100) {
            // exit after 100 tries
            std::cout << "warning: no valid direction found for pointInPolygon test" << std::endl;
            break;
        }

    } while (repeat);

    // in polygon if number of intersections is odd
    return intersections % 2 != 0 ? INSIDE : OUTSIDE;
}

vec2_t HEPolygon::getPointInPolygon() const {  // TODO: assert that adjacent edges are not parallel
    // get arbitrary point in polygon
    // see http://apodeline.free.fr/FAQ/CGAFAQ/CGAFAQ-3.html

    const VertexHandle va = findConvexPoint();
    const VertexHandle vb = nextVertex(va);
    const VertexHandle vc = prevVertex(va);

    // get convex point and neighbor points
    const vec2_t& a = point(va);
    const vec2_t& b = point(vb);
    const vec2_t& c = point(vc);

    real_t max_alpha = -1;
    vec2_t nearest_point(0);

    // test if other points are in the triangle abc
    for (VertexHandle v : vertices()) {
        if (v == va || v == vb || v == vc) {
            continue;
        }

        const vec2_t& p = point(v);

        // compute barycentric coordinates
        // see https://en.wikipedia.org/wiki/Barycentric_coordinate_system
        const real_t num_alpha = (b[1] - c[1]) * (p[0] - c[0]) + (c[0] - b[0]) * (p[1] - c[1]);
        const real_t num_beta = (c[1] - a[1]) * (p[0] - c[0]) + (a[0] - c[0]) * (p[1] - c[1]);
        const real_t div = 1 / ((b[1] - c[1]) * (a[0] - c[0]) + (c[0] - b[0]) * (a[1] - c[1]));

        const real_t alpha = num_alpha * div;
        const real_t beta = num_beta * div;

        // if p is in abc check if it is closer to a
        if (alpha >= 0 && beta >= 0 && alpha + beta < 1) {

            if (alpha > max_alpha) {
                max_alpha = alpha;
                nearest_point = p;
            }
        }
    }

    if (max_alpha < 0) {  // no point in abc
        return (a + b + c) / 3;
    } else {
        return (a + nearest_point) / 2;
    }
}

HEPolygon::VertexHandle HEPolygon::findConvexPoint() const {
    // get point on convex hull
    bool init = true;

    VertexHandle handle = 0;
    vec2_t pos(0);

    for (VertexHandle v : vertices()) {
        const vec2_t& p = point(v);

        if (init || p[0] < pos[0] || (p[0] == pos[0] && p[1] < pos[1])) {
            pos = p;
            handle = v;

            init = false;
        }
    }

    return handle;
}


template<typename Handle>
HEPolygonIterator<Handle>::HEPolygonIterator(const HEPolygon& poly, Handle start, bool ordered)
    : poly(poly), start(start), handle(start), ordered(ordered), is_end(false) {

    // find first valid handle
    if (handle < poly.points.size() && !poly.isValid(handle)) {
        if (ordered) {
            do {
                handle = poly.nextVertex(handle);  // works for both vertices and halfedges
            } while (!poly.isValid(handle));
        } else {
            operator++();
        }
    }
}

template<typename Handle>
HEPolygonIterator<Handle>& HEPolygonIterator<Handle>::operator++() {
    if (ordered) {

        // advance start until it is valid, but don't overtake handle
        while (!poly.isValid(start) && handle != start) {
            start = poly.nextVertex(start);
        }

        handle = poly.nextVertex(handle);
        if (handle == start) {
            is_end = true;
        }

    } else {

        // find next valid handle
        do {
            handle++;
        } while (handle < poly.points.size() && !poly.isValid(handle));

        if (handle == poly.points.size()) {
            is_end = true;
        }
    }

    return *this;
}

template<typename Handle>
HEPolygonIterator<Handle> HEPolygonIterator<Handle>::operator++(int) {
    HEPolygonIterator<Handle> it = *this;
    operator++();
    return it;
}

template<typename Handle>
HEPolygonIterator<Handle>& HEPolygonIterator<Handle>::operator+(std::size_t n) {
    for (std::size_t i = 0; i < n; i++) {
        if (is_end || (!ordered && handle >= poly.points.size())) {
            break;
        }
        operator++();
    }
    return *this;
}

template<typename Handle>
Handle HEPolygonIterator<Handle>::operator*() const {
    return handle;
}

template<typename Handle>
bool HEPolygonIterator<Handle>::operator==(const HEPolygonIterator& other) const {
    if (&poly != &other.poly || is_end != other.is_end) {
        return false;
    }
    return (is_end && other.is_end) || handle == other.handle;
}

template<typename Handle>
bool HEPolygonIterator<Handle>::operator!=(const HEPolygonIterator& other) const {
    return !operator==(other);
}

template<typename Handle>
void HEPolygonIterator<Handle>::toEnd() {
    // advance this iterator to the end
    is_end = true;
    if (!ordered) {
        handle = poly.points.size();
    }
}

template class HEPolygonIterator<std::size_t>;

}
