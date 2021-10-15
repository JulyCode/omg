#pragma once

#include <types.h>

namespace omg {

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
             typename S = typename std::conditional<std::is_same<Type, DefaultType>::value, T, Type>::type>

    S getValue(const vec2_t& point) const;

    vec2_t getGradient(const vec2_t& point) const;

    vec2_t computeGradient(const size2_t& idx) const;

    inline const AxisAlignedBoundingBox& getBoundingBox() const { return aabb; }
    inline const size2_t& getGridSize() const { return grid_size; }
    inline const vec2_t& getCellSize() const { return cell_size; }

    inline const T& grid(std::size_t i, std::size_t j) const { return grid(size2_t(i, j)); }
    inline T& grid(std::size_t i, std::size_t j) { return grid(size2_t(i, j)); }

    inline const T& grid(const size2_t& idx) const { return grid_values[linearIndex(idx)]; }
    inline T& grid(const size2_t& idx) { return grid_values[linearIndex(idx)]; }

    inline const std::vector<T>& grid() const { return grid_values; }
    inline std::vector<T>& grid() { return grid_values; }

    inline vec2_t getPoint(const size2_t& idx) const { return aabb.min + toVec2(idx) * cell_size; }

    inline std::size_t linearIndex(const size2_t& idx) const;
    inline size2_t gridIndex(std::size_t linear_idx) const;

protected:
    const AxisAlignedBoundingBox aabb;

    const size2_t grid_size;
    const vec2_t cell_size;

    std::vector<T> grid_values;

    template<typename S>
    inline S bilinearInterpolation(const S& f11, const S& f12, const S& f21, const S& f22, const vec2_t& factor) const;

    inline size2_t getSurroundingCell(const vec2_t& point, vec2_t& min, vec2_t& max) const;
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
template<typename Type, typename S>
S ScalarField<T>::getValue(const vec2_t& point) const {

    // if a non floating point type is implicitly used, show a warning
    static_assert(!std::is_same<Type, DefaultType>::value || std::is_floating_point<S>::value,
                  "implicit non floating point interpolation used");

    vec2_t min_corner(0), max_corner(0);
    const size2_t min_idx = getSurroundingCell(point, min_corner, max_corner);

    // get values (without bounds checks) and convert to interpolation type
    const S f11 = static_cast<S>(grid_values[min_idx[0]     +  min_idx[1]      * grid_size[0]]);
    const S f12 = static_cast<S>(grid_values[min_idx[0]     + (min_idx[1] + 1) * grid_size[0]]);
    const S f21 = static_cast<S>(grid_values[min_idx[0] + 1 +  min_idx[1]      * grid_size[0]]);
    const S f22 = static_cast<S>(grid_values[min_idx[0] + 1 + (min_idx[1] + 1) * grid_size[0]]);

    vec2_t factor = (point - min_corner) / (max_corner - min_corner);

    return bilinearInterpolation(f11, f12, f21, f22, factor);
}

template<typename T>
vec2_t ScalarField<T>::getGradient(const vec2_t& point) const {

    static_assert(std::is_convertible<T, real_t>::value, "gradient is only defined on scalar values");

    vec2_t min_corner(0), max_corner(0);
    const size2_t min_idx = getSurroundingCell(point, min_corner, max_corner);

    // get gradient values
    const vec2_t f11 = computeGradient(min_idx + size2_t(0, 0));
    const vec2_t f12 = computeGradient(min_idx + size2_t(0, 1));
    const vec2_t f21 = computeGradient(min_idx + size2_t(1, 0));
    const vec2_t f22 = computeGradient(min_idx + size2_t(1, 1));

    vec2_t factor = (point - min_corner) / (max_corner - min_corner);

    return bilinearInterpolation(f11, f12, f21, f22, factor);
}

template<typename T>
vec2_t ScalarField<T>::computeGradient(const size2_t& idx) const {
    // compute the gradient at this grid point using central difference if possible

    vec2_t distance(0);

    // check if backward difference is possible
    size2_t min_idx = idx;
    for (int i = 0; i < 2; i++) {
        if (min_idx[i] > 0) {
            min_idx[i]--;
            distance[i] += cell_size[i];
        }
    }

    // check if forward difference is possible
    size2_t max_idx = idx;
    for (int i = 0; i < 2; i++) {
        if (max_idx[i] < grid_size[i] - 1) {
            max_idx[i]++;
            distance[i] += cell_size[i];
        }
    }

    // get values (without bounds checks)
    const T f_min_x = grid_values[min_idx[0] + idx[1] * grid_size[0]];
    const T f_max_x = grid_values[max_idx[0] + idx[1] * grid_size[0]];
    const T f_min_y = grid_values[idx[0] + min_idx[1] * grid_size[0]];
    const T f_max_y = grid_values[idx[0] + max_idx[1] * grid_size[0]];

    assert(distance[0] != 0 && distance[1] != 0);

    vec2_t grad(0);
    grad[0] = static_cast<real_t>(f_max_x - f_min_x) / distance[0];
    grad[1] = static_cast<real_t>(f_max_y - f_min_y) / distance[1];

    return grad;
}

template<typename T>
inline std::size_t ScalarField<T>::linearIndex(const size2_t& idx) const {
    assert(idx[0] < grid_size[0] && idx[1] < grid_size[1]);

    return idx[0] + idx[1] * grid_size[0];
}

template<typename T>
inline size2_t ScalarField<T>::gridIndex(std::size_t linear_idx) const {
    assert(linear_idx < grid_values.size());

    return {linear_idx % grid_size[0], linear_idx / grid_size[0]};
}

template<typename T>
template<typename S>
inline S ScalarField<T>::bilinearInterpolation(const S& f11, const S& f12, const S& f21, const S& f22,
                                               const vec2_t& factor) const {

    const S v0 = f11 * (1 - factor[0]) * (1 - factor[1]);
    const S v1 = f12 * (1 - factor[0]) * factor[1];
    const S v2 = f21 * factor[0] * (1 - factor[1]);
    const S v3 = f22 * factor[0] * factor[1];

    return v0 + v1 + v2 + v3;
}

template<typename T>
inline size2_t ScalarField<T>::getSurroundingCell(const vec2_t& point, vec2_t& min, vec2_t& max) const {
    // calculates the minimum corner index and coordinates of the corners for the cell containing the point

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

    min = aabb.min + toVec2(min_idx) * cell_size;
    max = min + cell_size;

    return min_idx;
}


using BathymetryData = ScalarField<int16_t>;

}
