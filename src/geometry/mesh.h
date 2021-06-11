
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>

namespace omg {

struct MeshTraits : public OpenMesh::DefaultTraits {
    typedef OpenMesh::Vec2d Point;  // use two doubles as point coordinates
};

typedef OpenMesh::TriMesh_ArrayKernelT<MeshTraits> Mesh;

}
