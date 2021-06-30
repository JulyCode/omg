
#include "size_function.h"

namespace omg {

SizeFunction::SizeFunction(const AxisAlignedBoundingBox& aabb, const size2_t& grid_size)
    : ScalarField(aabb, grid_size) {}

SizeFunction::~SizeFunction() {}

bool SizeFunction::isTriangleGood(const vec2_t& v0, const vec2_t& v1, const vec2_t& v2) const {
    real_t s0 = getValue(v0);
    real_t s1 = getValue(v1);
    real_t s2 = getValue(v2);

    real_t min_size = std::min(s0, std::min(s1, s2));

    // TODO: best metric for triangle size?
    // use area for now
    real_t actual_size = toVec3(v1 - v0).cross(toVec3(v2 - v0)).norm() / 2;

    return actual_size < min_size;
}

}
