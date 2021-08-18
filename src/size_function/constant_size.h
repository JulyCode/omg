#pragma once

#include <size_function/size_function.h>

namespace omg {

class ConstantSize : public SizeFunction {
public:
    ConstantSize(const AxisAlignedBoundingBox& aabb, real_t constant_size);
};

}
