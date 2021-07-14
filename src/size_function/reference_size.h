#pragma once

#include <size_function/size_function.h>

namespace omg {

class AreaOfInterest {
public:
    AreaOfInterest(const vec2_t& center_pos, real_t inner_radius, real_t outer_radius, real_t resolution);

    real_t blendResolution(real_t current_resolution, const vec2_t& position) const;

private:
    // TODO: std::string name?
    vec2_t center_pos;

    real_t inner_radius;
    real_t outer_radius;

    real_t resolution;
};


struct Resolution {
    real_t coarsest;
    real_t finest;
    real_t coastal;

    std::vector<AreaOfInterest> aois;
};


class ReferenceSize : public SizeFunction {
public:
    ReferenceSize(const BathymetryData& data, const Resolution& resolution);

private:
    real_t calculateSize(const size2_t& idx, const BathymetryData& data, const Resolution& res) const;
};

}
