#pragma once

#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>

namespace omg {

struct MeshTraits : public OpenMesh::DefaultTraits {
    typedef OpenMesh::Vec3d Point;  // 2D vectors have problems when writing to a file
};

typedef OpenMesh::TriMesh_ArrayKernelT<MeshTraits> Mesh;

}
