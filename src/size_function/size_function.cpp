
#include "size_function.h"

#include <util.h>

namespace omg {

SizeFunction::SizeFunction(const AxisAlignedBoundingBox& aabb, const size2_t& grid_size)
    : ScalarField(aabb, grid_size), max(0) {}

SizeFunction::~SizeFunction() {}

bool SizeFunction::isTriangleGood(const vec2_t& v0, const vec2_t& v1, const vec2_t& v2) const {
    const real_t s0 = getValue(v0);
    const real_t s1 = getValue(v1);
    const real_t s2 = getValue(v2);

    const real_t min_size = std::min({s0, s1, s2});

    // maximum edge length in meters as metric for triangle size
    const real_t length0 = (v0 - v1).sqrnorm();
    const real_t length1 = (v0 - v2).sqrnorm();
    const real_t length2 = (v1 - v2).sqrnorm();
    const real_t max_length = std::sqrt(std::max({length0, length1, length2}));

    return max_length < min_size * 1.01;
}

}
