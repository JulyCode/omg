#include <iostream>

#include <geometry/polygon.h>
#include <io/poly_reader.h>
#include <triangulation/triangle_triangulator.h>

int main() {
    omg::Polygon poly = omg::io::readPoly("../../apps/medsea.poly");
    std::cout << "input polygon:" << std::endl;
    std::cout << "vertices: " << poly.getVertices().size() << std::endl;
    std::cout << "edges: " << poly.getEdges().size() << std::endl;

    omg::TriangleTriangulator tri;
    tri.generateMesh(poly);
}