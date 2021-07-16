#pragma once

#include <topology/scalar_field.h>

namespace omg {
namespace analysis {

template<typename T, typename S = T>  // TODO: refactor
inline ScalarField<S> difference(const ScalarField<T>& f1, const ScalarField<T>& f2) {

    // use highest resolution
    vec2_t highest_res(0);
    highest_res[0] = std::min(f1.getCellSize()[0], f2.getCellSize()[0]);
    highest_res[1] = std::min(f1.getCellSize()[1], f2.getCellSize()[1]);

    // compute intersection of bounding boxes
    const AxisAlignedBoundingBox& bb1 = f1.getBoundingBox();
    const AxisAlignedBoundingBox& bb2 = f2.getBoundingBox();

    AxisAlignedBoundingBox aabb;
    aabb.min = vec2_t(std::max(bb1.min[0], bb2.min[0]), std::max(bb1.min[1], bb2.min[1]));
    aabb.max = vec2_t(std::min(bb1.max[0], bb2.max[0]), std::min(bb1.max[1], bb2.max[1]));

    // calculate grid size and round
    const size2_t grid_size = toSize2((aabb.max - aabb.min) / highest_res + vec2_t(1.5));

    // create new scalar field
    ScalarField<S> diff(aabb, grid_size);

    #pragma omp parallel for
    for (std::size_t i = 0; i < grid_size[0]; i++) {
        for (std::size_t j = 0; j < grid_size[1]; j++) {

            const vec2_t pos = vec2_t(i, j) * diff.getCellSize() + aabb.min;
            // weird template issue
            diff.grid(i, j) = f1.template getValue<S>(pos) - f2.template getValue<S>(pos);
        }
    }

    return diff;
}

template<typename T>
inline T norm(const ScalarField<T>& field) {
    const size2_t grid_size = field.getGridSize();

    T sum = 0;

    #pragma omp parallel for reduction(+: sum)
    for (std::size_t i = 0; i < grid_size[0]; i++) {
        for (std::size_t j = 0; j < grid_size[1]; j++) {

            const T v = field.grid(i, j);
            sum += v * v;
        }
    }

    return std::sqrt(sum);
}

template<typename T>
inline std::vector<T> difference(const std::vector<T>& f1, const std::vector<T>& f2) {
    if (f1.size() != f2.size()) {
        throw std::runtime_error("Cannot compute difference between different size vectors");
    }

    std::vector<T> diff(f1.size());

    if (f1.size() > 1000000) {
        // parallelize
        #pragma omp parallel for
        for (std::size_t i = 0; i < f1.size(); i++) {
            diff[i] = f1[i] - f2[i];
        }
    } else {
        // sequential
        for (std::size_t i = 0; i < f1.size(); i++) {
            diff[i] = f1[i] - f2[i];
        }
    }

    return diff;
}

}
}
