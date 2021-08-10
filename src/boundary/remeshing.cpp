
#include "remeshing.h"

#include <iostream>

#include <geometry/line_intersection.h>

namespace omg {

static std::size_t collapse(HEPolygon& poly, const SizeFunction& size) {
    std::size_t count = 0;
    for (HEPolygon::HalfEdgeHandle heh : poly.halfEdges()) {

        const vec2_t& p1 = poly.startPoint(heh);
        const vec2_t& p2 = poly.endPoint(heh);
        const vec2_t& p3 = poly.endPoint(poly.nextHalfEdge(heh));

        const real_t target = size.getValue((p1 + p2) / 2);

        if (degreesToMeters((p2 - p1).norm()) < target) {
            poly.collapse(heh, 0);  // TODO: use center, but with back projection
            count++;
        }

        // fix degenerated areas
        if (degreesToMeters((p3 - p1).norm()) < 1) {
            poly.collapse(heh, 0);  // TODO: use center, but with back projection
            count++;
        }
    }
    std::cout << count << " collapses" << std::endl;
    return count;
}

void remesh(HEPolygon& poly, const SizeFunction& size) {
    while (collapse(poly, size) != 0) {}

    if (poly.hasSelfIntersection()) {
        std::cout << "self intersection" << std::endl;
    }
}

}
