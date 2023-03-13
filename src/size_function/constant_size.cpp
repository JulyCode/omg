
#include "constant_size.h"

namespace omg {

ConstantSize::ConstantSize(const AxisAlignedBoundingBox& aabb, real_t constant_size)
    : SizeFunction(aabb, size2_t(2, 2)) {

    grid(0, 0) = constant_size;
    grid(0, 1) = constant_size;
    grid(1, 0) = constant_size;
    grid(1, 1) = constant_size;

    max = constant_size;
}

}
