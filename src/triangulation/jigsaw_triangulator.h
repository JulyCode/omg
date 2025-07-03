#pragma once

#include <triangulation/triangulator.h>

namespace omg {

class JigsawTriangulator : public Triangulator {
public:
    JigsawTriangulator();

    void generateMesh(const Boundary& boundary, const SizeFunction& size, Mesh& out_mesh, bool keep_boundary = true) override;
};

}
