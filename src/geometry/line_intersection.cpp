
#include "line_intersection.h"

#include <iostream>
#include <cmath>

namespace omg {

static void computeValues(const LineSegment& l1, const LineSegment& l2, real_t& numT, real_t& numU, real_t& den) {
    const vec2_t& p1 = l1.first;
    const vec2_t& p2 = l1.second;
    const vec2_t& p3 = l2.first;
    const vec2_t& p4 = l2.second;

    // see https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection
    numT = (p1[0] - p3[0]) * (p3[1] - p4[1]) - (p1[1] - p3[1]) * (p3[0] - p4[0]);
    numU = (p2[0] - p1[0]) * (p1[1] - p3[1]) - (p2[1] - p1[1]) * (p1[0] - p3[0]);
    den = (p1[0] - p2[0]) * (p3[1] - p4[1]) - (p1[1] - p2[1]) * (p3[0] - p4[0]);
}

// checks if p is on the segment ls, assuming they are collinear
static bool onSegment(const LineSegment& ls, const vec2_t& p) {
    return p[0] <= std::max(ls.first[0], ls.second[0]) && p[0] >= std::min(ls.first[0], ls.second[0]) &&
           p[1] <= std::max(ls.first[1], ls.second[1]) && p[1] >= std::min(ls.first[1], ls.second[1]);
}


// internal
static bool collinear(const LineSegment& l1, const LineSegment& l2, real_t numT, real_t den) {
    if (den == 0 && numT == 0) {
        // are segments overlapping?
        return onSegment(l1, l2.first) || onSegment(l1, l2.second) || onSegment(l2, l1.first);
    }
    return false;
}

bool collinear(const LineSegment& l1, const LineSegment& l2) {
    real_t numT, numU, den;
    computeValues(l1, l2, numT, numU, den);

    return collinear(l1, l2, numT, den);
}


// internal
static bool lineIntersection(const LineSegment& l1, const LineSegment& l2, real_t numT, real_t numU, real_t den) {
    if (den == 0) {  // parallel
        return collinear(l1, l2, numT, den);  // no intersection if parallel but not collinear
    }

    // intersection if t <= 1, t >= 0, u <= 1, u >= 0
    return std::abs(numT) <= std::abs(den) && numT * den >= 0 &&
           std::abs(numU) <= std::abs(den) && numU * den >= 0;
}

bool lineIntersection(const LineSegment& l1, const LineSegment& l2) {
    real_t numT, numU, den;
    computeValues(l1, l2, numT, numU, den);

    return lineIntersection(l1, l2, numT, numU, den);
}


// compute factor t in [0, 1] of l1
std::optional<real_t> lineIntersectionFactor(const LineSegment& l1, const LineSegment& l2) {
    real_t numT, numU, den;
    computeValues(l1, l2, numT, numU, den);

    if (!lineIntersection(l1, l2, numT, numU, den)) {
        return {};
    }

    // if lines are almost parallel
    if (std::abs(den) < 0.0001) {
        std::cout << "almost parallel lines" << std::endl;

        // approximate intersection
        // TODO: use gradient descent?
        const real_t length1 = (l1.second - l1.first).sqrnorm();
        const real_t length2 = (l2.second - l2.first).sqrnorm();

        if (length1 > length2) {
            const real_t avg_dist = ((l2.first + l2.second) / 2 - l1.first).sqrnorm();
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
