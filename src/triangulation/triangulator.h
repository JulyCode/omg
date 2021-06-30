#pragma once

#include <geometry/polygon.h>
#include <geometry/mesh.h>
#include <size_function/size_function.h>

namespace omg {

class Triangulator {  // TODO: does triangulation work with spherical coordinates?
public:
    Triangulator() {}
    virtual ~Triangulator() {}

    virtual void generateMesh(const Polygon& outline, const SizeFunction& size, Mesh& out_mesh) = 0;

protected:
    void restrictToInt(const Polygon& outline) const;
};

}
