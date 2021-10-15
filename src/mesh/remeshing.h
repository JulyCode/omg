#pragma once

#include <mesh/mesh.h>
#include <size_function/size_function.h>

namespace omg {

class IsotropicRemeshing {
public:
    explicit IsotropicRemeshing(const SizeFunction& size, real_t min_size_factor = 0.6, real_t max_size_factor = 1.3);

    void remesh(Mesh& mesh, unsigned int iterations = 10) const;

    static int computeOptimalValence(const OpenMesh::SmartVertexHandle& vh, const Mesh& mesh);

private:
    const SizeFunction& size;
    const real_t min_size_factor;
    const real_t max_size_factor;

    void splitEdges(Mesh& mesh) const;

    void collapseEdges(Mesh& mesh) const;
    bool isCollinear(const Mesh& mesh, const OpenMesh::SmartHalfedgeHandle& heh,
                     const OpenMesh::SmartVertexHandle& vh) const;

    void equalizeValences(Mesh& mesh) const;

    void smoothVertices(Mesh& mesh) const;
};

}
