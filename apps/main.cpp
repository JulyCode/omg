#include <iostream>

#include <geometry/polygon.h>
#include <io/poly_reader.h>
#include <triangulation/triangle_triangulator.h>

int main() {
    omg::Polygon poly = omg::io::readPoly("../../apps/medsea.poly");
    std::cout << "input polygon:" << std::endl;
    std::cout << "vertices: " << poly.getVertices().size() << std::endl;
    std::cout << "edges: " << poly.getEdges().size() << std::endl << std::endl;

    omg::Mesh mesh;
    omg::TriangleTriangulator tri;
    tri.generateMesh(poly, mesh);

    std::cout << "output mesh:" << std::endl;
    std::cout << "vertices: " << mesh.n_vertices() << std::endl;
    std::cout << "triangles: " << mesh.n_faces() << std::endl;
}