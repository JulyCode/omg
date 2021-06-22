#pragma once

#include <geometry/types.h>

#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>

namespace omg {

struct MeshTraits : public OpenMesh::DefaultTraits {
    typedef vec_t Point;
    typedef real_t Scalar;
};

typedef OpenMesh::TriMesh_ArrayKernelT<MeshTraits> Mesh;

}
