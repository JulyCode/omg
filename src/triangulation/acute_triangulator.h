#pragma once

#include <triangulation/triangulator.h>

// context from triangle_api.h
struct context_t;
typedef context_t context;

namespace omg {

class ACuteTriangulator : public Triangulator {
public:
    ACuteTriangulator();
    ~ACuteTriangulator();

    void generateMesh(const Polygon& outline, Mesh& out_mesh);

private:
    context* ctx;

    void check(int status_code);

    static int triunsuitable(double* v1, double* v2, double* v3, double area);
};

}
