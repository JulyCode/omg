#pragma once

#include <triangulation/triangulator.h>

namespace omg {

class TriangleTriangulator : public Triangulator {
public:
    TriangleTriangulator();
    ~TriangleTriangulator();

    void generateMesh(const Polygon& outline, Mesh& out_mesh);

private:
    static int triunsuitable(double* v1, double* v2, double* v3, double area);
};

}
