#pragma once

#include <geometry/types.h>

namespace omg {

struct AxisAlignedBoundingBox {
    vec2_t min, max;
};

template<typename T>
class ScalarField {
public:

    // the bounding box aligns with the outermost sample points
    // so sample points lie at the corners of cells, not in the center
    ScalarField(const AxisAlignedBoundingBox& aabb, const size2_t& grid_size);

    virtual ~ScalarField() {}

    // specify type only used for interpolation
    template<typename S = T>
    S getValue(const vec2_t& point) const;

    inline const AxisAlignedBoundingBox& getBoundingBox() const { return aabb; }
    inline const size2_t& getGridSize() const { return grid_size; }

    inline const T& grid(std::size_t i, std::size_t j) const { return grid(size2_t(i, j)); }
    inline T& grid(std::size_t i, std::size_t j) { return grid(size2_t(i, j)); }

    inline const T& grid(const size2_t& idx) const { return grid_values[linearIndex(idx)]; }
    inline T& grid(const size2_t& idx) { return grid_values[linearIndex(idx)]; }

    inline const std::vector<T>& grid() const { return grid_values; }
    inline std::vector<T>& grid() { return grid_values; }

    inline std::size_t linearIndex(const size2_t& idx) const;

private:
    const AxisAlignedBoundingBox aabb;

    const size2_t grid_size;
    const vec2_t cell_size;

    std::vector<T> grid_values;
};


// ---------------------- implementation ----------------------

template<typename T>
ScalarField<T>::ScalarField(const AxisAlignedBoundingBox& aabb, const size2_t& grid_size)
    : aabb(aabb), grid_size(grid_size), cell_size((aabb.max - aabb.min) / (grid_size - vec2_t(1))) {

    if (grid_size[0] <= 1 || grid_size[1] <= 1) {
        throw std::runtime_error("grid size must be at least 2x2");
    }
    grid_values.resize(grid_size[0] * grid_size[1]);
}

template<typename T>
template<typename S>
S ScalarField<T>::getValue(const vec2_t& point) const {  // TODO: maybe use different interpolations?
    // bi-linear interpolation according to https://en.wikipedia.org/wiki/Bilinear_interpolation

    if (point[0] < aabb.min[0] || point[1] < aabb.min[1] || point[0] > aabb.max[0] || point[1] > aabb.max[1]) {
        throw std::runtime_error("Trying to access scalar field out of bounds");
    }

    // calculate min index including border case
    size2_t min_idx = toSize2((point - aabb.min) / cell_size);
    if (min_idx[0] == grid_size[0] - 1) {
        min_idx[0]--;
    }
    if (min_idx[1] == grid_size[1] - 1) {
        min_idx[1]--;
    }

    const vec2_t min_corner = aabb.min + toVec2(min_idx) * cell_size;
    const vec2_t max_corner = min_corner + cell_size;

    // get values (without bounds checks) and convert to interpolation type
    const S f11 = static_cast<S>(grid_values[ min_idx[0]      * grid_size[1] + min_idx[1]    ]);
    const S f21 = static_cast<S>(grid_values[(min_idx[0] + 1) * grid_size[1] + min_idx[1]    ]);
    const S f12 = static_cast<S>(grid_values[ min_idx[0]      * grid_size[1] + min_idx[1] + 1]);
    const S f22 = static_cast<S>(grid_values[(min_idx[0] + 1) * grid_size[1] + min_idx[1] + 1]);

    // compute coefficients for interpolation
    const real_t div = (max_corner[0] - min_corner[0]) * (max_corner[1] - min_corner[1]);
    const vec2_t d1 = max_corner - point;
    const vec2_t d2 = point - min_corner;

    return (f11 * d1[0] * d1[1] + f21 * d2[0] * d1[1] + f12 * d1[0] * d2[1] + f22 * d2[0] * d2[1]) / div;
}

template<typename T>
inline std::size_t ScalarField<T>::linearIndex(const size2_t& idx) const {
        if (idx[0] >= grid_size[0] || idx[1] >= grid_size[1]) {
            const std::string s1 = std::to_string(idx[0]) + ", " + std::to_string(idx[1]);
            const std::string s2 = std::to_string(grid_size[0]) + ", " + std::to_string(grid_size[1]);
            throw std::out_of_range("Index (" + s1 + ") is out of range (" + s2 + ")");
        }
        return idx[0] * grid_size[1] + idx[1];
    }

}
