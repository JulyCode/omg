#pragma once

#include <optional>

#include <types.h>

namespace omg {

using LineSegment = std::pair<const vec2_t&, const vec2_t&>;

bool collinear(const LineSegment& l1, const LineSegment& l2);

bool lineIntersection(const LineSegment& l1, const LineSegment& l2);

std::optional<real_t> lineIntersectionFactor(const LineSegment& l1, const LineSegment& l2);

std::optional<vec2_t> lineIntersectionPoint(const LineSegment& l1, const LineSegment& l2);

}
