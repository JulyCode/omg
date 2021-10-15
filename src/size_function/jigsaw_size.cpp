
#include "jigsaw_size.h"

namespace omg {

JigsawSizeFunction::JigsawSizeFunction(const SizeFunction& size)
    : grid_size(size.getGridSize()), aabb(size.getBoundingBox()) {

    x_buffer.reserve(grid_size[0]);
    y_buffer.reserve(grid_size[1]);
    value_buffer.reserve(grid_size[0] * grid_size[1]);

    for (std::size_t x = 0; x < grid_size[0]; x++) {

        const vec2_t p = size.getPoint({x, 0});
        x_buffer.push_back(p[0]);
    }
    for (std::size_t y = 0; y < grid_size[1]; y++) {

        const vec2_t p = size.getPoint({0, y});
        y_buffer.push_back(p[1]);
    }

    for (std::size_t x = 0; x < grid_size[0]; x++) {
        for (std::size_t y = 0; y < grid_size[1]; y++) {

            value_buffer.push_back(size.grid(x, y));
        }
    }

    jigsaw_init_msh_t(&h_fun);

    h_fun._flags = JIGSAW_EUCLIDEAN_GRID;

    h_fun._value._data = value_buffer.data();
    h_fun._value._size = value_buffer.size();

    h_fun._xgrid._data = x_buffer.data();
    h_fun._xgrid._size = x_buffer.size();

    h_fun._ygrid._data = y_buffer.data();
    h_fun._ygrid._size = y_buffer.size();
}

void JigsawSizeFunction::setGradientLimit(real_t limit) {
    if (slope_buffer.empty()) {
        slope_buffer.resize(value_buffer.size());

        h_fun._slope._data = slope_buffer.data();
        h_fun._slope._size = slope_buffer.size();
    }

    for (::fp32_t& v : slope_buffer) {
        v = limit;
    }
}

void JigsawSizeFunction::toSizeFunction(SizeFunction& size) const {
    if (grid_size != size.getGridSize()) {
        throw std::runtime_error("Wrong grid size");
    }
    if (aabb.min != size.getBoundingBox().min || aabb.max != size.getBoundingBox().max) {
        throw std::runtime_error("Wrong bounding box");
    }

    for (std::size_t x = 0; x < grid_size[0]; x++) {
        for (std::size_t y = 0; y < grid_size[1]; y++) {

            size.grid(x, y) = value_buffer[x * grid_size[1] + y];
        }
    }
}

}
