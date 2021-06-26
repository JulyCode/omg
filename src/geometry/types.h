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
    return s < std::numeric_limits<int>::max();
}

}
