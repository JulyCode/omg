#pragma once

#include <types.h>

#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>

namespace omg {

struct MeshTraits : public OpenMesh::DefaultTraits {
    typedef vec3_t Point;
    typedef real_t Scalar;
};

typedef OpenMesh::TriMesh_ArrayKernelT<MeshTraits> Mesh;

}
