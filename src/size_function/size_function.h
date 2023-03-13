#pragma once

#include <topology/scalar_field.h>

namespace omg {

class SizeFunction : public ScalarField<real_t> {
public:
    SizeFunction(const AxisAlignedBoundingBox& aabb, const size2_t& grid_size);

    SizeFunction(const ScalarField<real_t>& scalar_field);
    SizeFunction(ScalarField<real_t>&& scalar_field);

    // return true if the triangle (v0, v1, v2) satisfies the size constraints
    bool isTriangleGood(const vec2_t& v0, const vec2_t& v1, const vec2_t& v2) const;

    void find_max();
    inline real_t getMax() const { return max; }

protected:
    real_t max;
};

}
