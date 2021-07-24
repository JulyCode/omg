#pragma once

#include <OpenMesh/Core/Geometry/VectorT.hh>

namespace omg {

using real_t = double;

using vec3_t = OpenMesh::Vec3d;
using vec2_t = OpenMesh::Vec2d;

using size2_t = OpenMesh::VectorT<std::size_t, 2>;


inline vec3_t toVec3(const vec2_t& v) {
    return vec3_t(v[0], v[1], 0);
}

inline vec2_t toVec2(const vec3_t& v) {
    return vec2_t(v[0], v[1]);
}

inline vec2_t toVec2(const size2_t& v) {
    return vec2_t(static_cast<real_t>(v[0]), static_cast<real_t>(v[1]));
}

inline size2_t toSize2(const vec2_t& v) {
    if (v[0] < 0 || v[1] < 0) {
        throw std::range_error("Cannot convert negative vec2_t to size2_t");
    }
    return size2_t(static_cast<std::size_t>(v[0]), static_cast<std::size_t>(v[1]));
}


constexpr bool fitsInt(std::size_t s) {
    return s < static_cast<std::size_t>(std::numeric_limits<int>::max());
}


constexpr real_t EARTH_RADIUS = 6'371'009;
// constexpr real_t EARTH_RADIUS = 6367500;

// C++20 would have a better way of doing this,
// but I still don't want to use the C-style macro
constexpr real_t PI = M_PI;

template<typename T>
constexpr T toRadians(T value) {
    return value * PI / 180.0;
}

template<typename T>
constexpr T toDegrees(T value) {
    return value * 180.0 / PI;
}

// only works as an approximation for distances,
// does not consider position on earth
constexpr real_t degreesToMeters(real_t value) {
    return toRadians(value) * EARTH_RADIUS;
}

// only works as an approximation for distances,
// does not consider position on earth
constexpr real_t metersToDegrees(real_t value) {
    return toDegrees(value) / EARTH_RADIUS;
}

inline real_t geoDistance(vec2_t p1, vec2_t p2) {
    // according to https://en.wikipedia.org/wiki/Geographical_distance
    const real_t d_lon = p2[0] - p1[0];
    const real_t d_lat = p2[1] - p1[1];
    const real_t mean_lat = (p1[1] + p2[1]) / 2;
    const real_t lon = std::cos(mean_lat) * d_lon;

    return EARTH_RADIUS * std::sqrt(d_lat * d_lat + lon * lon);
}


struct AxisAlignedBoundingBox {
    vec2_t min, max;

    inline vec2_t center() const {
        return (min + max) / 2;
    }
};

}
