
#include "simplification.h"

#include <iostream>

#include <geometry/line_intersection.h>

namespace omg {

static std::size_t collapse(HEPolygon& poly, const SizeFunction& size) {
    std::size_t count = 0;
    for (HEPolygon::HalfEdgeHandle heh : poly.halfEdges()) {

        if (poly.isDegenerated()) {
            break;
        }

        const vec2_t& p1 = poly.startPoint(heh);
        const vec2_t& p2 = poly.endPoint(heh);
        const vec2_t& p3 = poly.endPoint(poly.nextHalfEdge(heh));

        const real_t target = size.getValue((p1 + p2) / 2);

        if (degreesToMeters((p2 - p1).norm()) < target) {
            poly.collapse(heh, 0);
            count++;
        }

        // fix degenerated areas
        if (degreesToMeters((p3 - p1).norm()) < 1) {  // threshold dot product um spitze winkel zu entfernen
            poly.collapse(heh, 0);
            count++;
        }
    }
    return count;
}

void simplifyPolygon(HEPolygon& poly, const SizeFunction& size) {
    while (collapse(poly, size) != 0) {}
}

}
