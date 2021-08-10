
#include "line_intersection.h"

#include <iostream>

namespace omg {

bool lineIntersection(const LineSegment& l1, const LineSegment& l2) {
    const vec2_t& p1 = l1.first;
    const vec2_t& p2 = l1.second;
    const vec2_t& p3 = l2.first;
    const vec2_t& p4 = l2.second;

    const real_t numT = (p1[0] - p3[0]) * (p3[1] - p4[1]) - (p1[1] - p3[1]) * (p3[0] - p4[0]);
    const real_t numU = (p2[0] - p1[0]) * (p1[1] - p3[1]) - (p2[1] - p1[1]) * (p1[0] - p3[0]);
    const real_t den = (p1[0] - p2[0]) * (p3[1] - p4[1]) - (p1[1] - p2[1]) * (p3[0] - p4[0]);

    // no intersection if t > 1, t < 0, u > 1, u < 0
    const bool positiv = den > 0;
    if (((numT - den > 0) == positiv) || ((numT > 0) != positiv) ||
        ((numU - den > 0) == positiv) || ((numU > 0) != positiv)) {
        return false;
    }
    return true;
}

std::optional<real_t> lineIntersectionFactor(const LineSegment& l1, const LineSegment& l2) {
    // compute factor t in [0, 1] of l1
    // see https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection

    const vec2_t& p1 = l1.first;
    const vec2_t& p2 = l1.second;
    const vec2_t& p3 = l2.first;
    const vec2_t& p4 = l2.second;

    const real_t numT = (p1[0] - p3[0]) * (p3[1] - p4[1]) - (p1[1] - p3[1]) * (p3[0] - p4[0]);
    const real_t numU = (p2[0] - p1[0]) * (p1[1] - p3[1]) - (p2[1] - p1[1]) * (p1[0] - p3[0]);
    const real_t den = (p1[0] - p2[0]) * (p3[1] - p4[1]) - (p1[1] - p2[1]) * (p3[0] - p4[0]);

    // no intersection if t > 1, t < 0, u > 1, u < 0
    const bool positiv = den > 0;
    if (((numT - den > 0) == positiv) || ((numT > 0) != positiv) ||
        ((numU - den > 0) == positiv) || ((numU > 0) != positiv)) {
        return {};
    }

    // if lines are almost parallel
    if (std::abs(den) < 0.0001) {
        std::cout << "almost parallel lines" << std::endl;

        // approximate intersection
        // TODO: use gradient descent?
        const real_t length1 = (p2 - p1).sqrnorm();
        const real_t length2 = (p4 - p3).sqrnorm();

        if (length1 > length2) {
            const real_t avg_dist = ((p3 + p4) / 2 - p1).sqrnorm();
            return std::clamp(avg_dist / length1, 0.0, 1.0);
        } else {
            return 0.5;
        }
    }

    return numT / den;
}

std::optional<vec2_t> lineIntersectionPoint(const LineSegment& l1, const LineSegment& l2) {
    std::optional<real_t> t = lineIntersectionFactor(l1, l2);
    if (!t) {
        return {};
    }

    return l1.first + (*t) * (l1.second - l1.first);
}

}
