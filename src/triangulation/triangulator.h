#pragma once

#include <geometry/polygon.h>
#include <geometry/mesh.h>

namespace omg {

class Triangulator {
public:
    virtual void generateMesh(const Polygon& outline, Mesh& out_mesh) = 0;

protected:
    void restrictToInt(const Polygon& outline) const;
};

}
