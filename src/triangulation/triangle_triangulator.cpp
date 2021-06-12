
#include "triangle_triangulator.h"

#include <limits>

#include <Triangle/jrs_triangle.h>
#include <triangulation/triangle_helper.h>

namespace omg {

TriangleTriangulator::TriangleTriangulator() {
    // init callback function to interact with Triangle
    jrs::set_triunsuitable_callback(triunsuitable);
}

TriangleTriangulator::~TriangleTriangulator() {}

void TriangleTriangulator::generateMesh(const Polygon& outline, Mesh& mesh) {
    const std::size_t num_vertices = outline.getVertices().size();
    const std::size_t num_edges = outline.getEdges().size();

    // Triangle only uses int as size type
    if (num_vertices == 0 || num_vertices >= std::numeric_limits<int>::max()) {
        throw std::runtime_error("Invalid number of vertices in outline");
    }
    if (num_edges == 0 || num_edges >= std::numeric_limits<int>::max()) {
        throw std::runtime_error("Invalid number of edges in outline");
    }

    TriangleIn<jrs::triangulateio> in(outline);
    TriangleOut<jrs::triangulateio> out;

    // Q: no console output
    // z: use zero based indexing
    // B: don't output boundary information
    // P: don't output segment information
    // u: use triunsuitable quality check
    // p: triangulate a Planar Straight Line Graph (.poly file)
    // D: conforming Delaunay
    // a0.5: area constraint of 0.5
    // q25: minimum angle of 25 degrees, maximum 180 - 2 * 25
    char args[] = "zBPupDa0.5q25";
    jrs::triangulate(args, &in.io, &out.io, nullptr);

    out.toMesh(mesh);
}

int TriangleTriangulator::triunsuitable(double* v1, double* v2, double* v3, double area) {
    return 0;
}

}
