#pragma once

#include <OpenMesh/Core/Geometry/VectorT.hh>

namespace omg {

using real_t = double;

using vec_t = OpenMesh::Vec3d;


inline bool fitsInt(std::size_t s) {
    return s < std::numeric_limits<int>::max();
}

}
