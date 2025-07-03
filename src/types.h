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

class AxisAlignedBoundingBox {
public:
    AxisAlignedBoundingBox() : min(std::numeric_limits<real_t>::max(), std::numeric_limits<real_t>::max()), max(-std::numeric_limits<real_t>::max(), -std::numeric_limits<real_t>::max()) {}
    
    AxisAlignedBoundingBox(const vec2_t& min, const vec2_t& max) : min(min), max(max) {}

    AxisAlignedBoundingBox(const vec2_t& point) : min(point), max(point) {}

    vec2_t size() const {
        return max - min;
    }

    vec2_t center() const {
        return (min + max) / 2;
    }

    real_t area() const {
        return (max[0] - min[0]) * (max[1] - min[1]);
    }

    bool is_empty() const {
        return min[0] > max[0] || min[1] > max[1];
    }

    AxisAlignedBoundingBox& operator+=(const AxisAlignedBoundingBox& other) {
        min[0] = std::min(min[0], other.min[0]);
        min[1] = std::min(min[1], other.min[1]);
        max[0] = std::max(max[0], other.max[0]);
        max[1] = std::max(max[1], other.max[1]);
        return *this;
    }

    AxisAlignedBoundingBox operator+(const AxisAlignedBoundingBox& other) const {
        AxisAlignedBoundingBox aabb = *this;
        aabb += other;
        return aabb;
    }

public:
    vec2_t min;
    vec2_t max;
};

}
