
#include "reference_size.h"

#include <iostream>
#include <chrono>

namespace omg {

AreaOfInterest::AreaOfInterest(const vec2_t& center_pos, real_t inner_radius, real_t outer_radius, real_t resolution)
    : center_pos(center_pos), inner_radius(inner_radius), outer_radius(outer_radius), resolution(resolution) {}

real_t AreaOfInterest::blendResolution(real_t current_resolution, const vec2_t& position) const {
    // according to reference, could be calculated in a better way
    const real_t distance = (position - center_pos).norm();

    if (distance < inner_radius) {
        return std::min(current_resolution, resolution);

    } else if (distance > outer_radius) {
        return current_resolution;

    } else {
        // interpolate between current and target resolution (actually between resolution and coastal, but why?)
        const real_t factor = (distance - inner_radius) / (outer_radius - inner_radius);
        const real_t target_res = (1 - factor) * resolution + factor * current_resolution;

        return std::min(current_resolution, target_res);
    }
}


ReferenceSize::ReferenceSize(const BathymetryData& data, const Resolution& resolution)
    : SizeFunction(data.getBoundingBox(), data.getGridSize()) {

    auto begin = std::chrono::high_resolution_clock::now();

    #pragma omp parallel for
    for (std::size_t i = 0; i < grid_size[0]; i++) {
        for (std::size_t j = 0; j < grid_size[1]; j++) {

            const size2_t idx(i, j);
            grid(idx) = calculateSize(idx, data, resolution);
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
	std::cout << "Took: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << std::endl;
}

real_t ReferenceSize::calculateSize(const size2_t& idx, const BathymetryData& data, const Resolution& res) const {

    const vec2_t position = toVec2(idx) * getCellSize() + getBoundingBox().min;
    real_t depth = -static_cast<real_t>(data.grid(idx));
    const real_t gradient = data.computeGradient(idx).norm() / degreesToMeters(1.0);

    // code from reference

    if (depth < -500.0) {
        depth = -depth;  // flip heights above 500 m, why?
    }

    const real_t factor = res.coarsest / 200;  // is also cfl_glob

    real_t min_depth = 0.1 * (res.finest * res.finest) / (factor * factor);
    if (depth < min_depth) {
        depth = min_depth;  // clamp depth to below 10 meters?
    }

    const real_t c_gr = factor * 0.02;

    const real_t condition = factor * std::sqrt(9.81 * depth);

    real_t size = 2 * res.coarsest;
    size = std::min(size, std::max((c_gr * depth) / gradient, res.coastal));
    size = std::min(size, std::max(condition, res.coastal));

    // blend priority areas
    for (const AreaOfInterest& aoi : res.aois) {
        size = std::max(aoi.blendResolution(size, position), condition);  // why limit this?
    }

    return size;
}

}
