#pragma once

#include <boundary/boundary.h>
#include <mesh/mesh.h>
#include <size_function/size_function.h>

namespace omg {

class Triangulator {  // TODO: does triangulation work with spherical coordinates?
public:
    Triangulator() {}
    virtual ~Triangulator() {}

    virtual void generateMesh(const Boundary& boundary, const SizeFunction& size, Mesh& out_mesh, bool keep_boundary = true) = 0;
};

}
