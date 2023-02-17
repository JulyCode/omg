#pragma once

#include <triangulation/triangulator.h>

// context from triangle_api.h
struct context_t;
typedef context_t context;

namespace omg {

class ACuteTriangulator : public Triangulator {
public:
    explicit ACuteTriangulator(real_t min_angle = 25, real_t max_angle = 120);
    ~ACuteTriangulator();

    void generateMesh(const Boundary& boundary, const SizeFunction& size, Mesh& out_mesh) override;

private:
    static const SizeFunction* size_function;

    context* ctx;

    void check(int status_code) const;

    static int triunsuitable(double* v1, double* v2, double* v3, double area);
};

}
