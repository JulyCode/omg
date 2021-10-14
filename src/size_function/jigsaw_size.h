#pragma once

#include <size_function/size_function.h>

#include <jigsaw/inc/lib_jigsaw.h>

namespace omg {

class JigsawSizeFunction {
public:
    explicit JigsawSizeFunction(const SizeFunction& size);

    void setGradientLimit(real_t limit);

    void toSizeFunction(SizeFunction& size) const;

    inline jigsaw_msh_t& getJigsawMesh() { return h_fun; }

private:
    const size2_t grid_size;
    const AxisAlignedBoundingBox aabb;

    jigsaw_msh_t h_fun;

    std::vector<::real_t> x_buffer;
    std::vector<::real_t> y_buffer;
    std::vector<::fp32_t> value_buffer;

    std::vector<::fp32_t> slope_buffer;
};

}
