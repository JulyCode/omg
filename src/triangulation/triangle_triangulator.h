#pragma once

#include <triangulation/triangulator.h>

namespace omg {

class TriangleTriangulator : Triangulator {
public:
    TriangleTriangulator();
    ~TriangleTriangulator();

    void generateMesh(const Polygon& outline);

private:
    static int triunsuitable(double* v1, double* v2, double* v3, double area);
};

}
