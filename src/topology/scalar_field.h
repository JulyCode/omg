#pragma once

#include <geometry/types.h>

namespace omg {

struct AxisAlignedBoundingBox {
    vec2_t min, max;
};

// template type to check if the interpolation type was set explicitly or implicitly
struct DefaultType {};

template<typename T>
class ScalarField {
public:

    // the bounding box aligns with the outermost sample points
    // so sample points lie at the corners of cells, not in the center
    ScalarField(const AxisAlignedBoundingBox& aabb, const size2_t& grid_size);

    virtual ~ScalarField() {}

    // specify type only used for interpolation
    template<typename Type = DefaultType,

             // real interpolation type to replace DefaultType
             typename S = typename std::conditional<std::is_same<Type, DefaultType>::value, T, Type>::type,
             // if a non floating point type is implicitly used, show a warning
             bool type_warning = std::is_same<Type, DefaultType>::value && !std::is_floating_point<S>::value>

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

    template<typename S>
    inline S bilinearInterpolation(const S& f11, const S& f12, const S& f21, const S& f22, vec2_t factor) const;
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
template<typename Type, typename S, bool type_warning>
S ScalarField<T>::getValue(const vec2_t& point) const {

    static_assert(!type_warning, "implicit non floating point interpolation used");

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
    const S f11 = static_cast<S>(grid_values[min_idx[0]     +  min_idx[1]      * grid_size[0]]);
    const S f21 = static_cast<S>(grid_values[min_idx[0] + 1 +  min_idx[1]      * grid_size[0]]);
    const S f12 = static_cast<S>(grid_values[min_idx[0]     + (min_idx[1] + 1) * grid_size[0]]);
    const S f22 = static_cast<S>(grid_values[min_idx[0] + 1 + (min_idx[1] + 1) * grid_size[0]]);

    vec2_t factor = (point - min_corner) / (max_corner - min_corner);

    return bilinearInterpolation(f11, f12, f21, f22, factor);
}

template<typename T>
inline std::size_t ScalarField<T>::linearIndex(const size2_t& idx) const {
    if (idx[0] >= grid_size[0] || idx[1] >= grid_size[1]) {
        const std::string s1 = std::to_string(idx[0]) + ", " + std::to_string(idx[1]);
        const std::string s2 = std::to_string(grid_size[0]) + ", " + std::to_string(grid_size[1]);
        throw std::out_of_range("Index (" + s1 + ") is out of range (" + s2 + ")");
    }
    return idx[0] + idx[1] * grid_size[0];
}

template<typename T>
template<typename S>
inline S ScalarField<T>::bilinearInterpolation(const S& f11, const S& f12, const S& f21, const S& f22,
                                               vec2_t factor) const {

    const S v0 = f11 * (1 - factor[0]) * (1 - factor[1]);
    const S v1 = f21 * factor[0] * (1 - factor[1]);
    const S v2 = f12 * (1 - factor[0]) * factor[1];
    const S v3 = f22 * factor[0] * factor[1];

    return v0 + v1 + v2 + v3;
}

}
