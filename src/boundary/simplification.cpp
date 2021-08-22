
#include "simplification.h"

#include <util.h>
#include <geometry/line_intersection.h>

namespace omg {

static const real_t SMALLEST_ANGLE_COS = std::cos(toRadians(15.0));

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

        bool remove = degreesToMeters((p2 - p1).norm()) < target;

        // fix degenerated areas
        remove |= degreesToMeters((p3 - p1).norm()) < 1;  // TODO: use geoDistance?

        // remove small angles
        remove |= (p1 - p2).normalized().dot((p3 - p2).normalized()) > SMALLEST_ANGLE_COS;

        if (remove) {
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
