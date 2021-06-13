#include <iostream>

#include <geometry/polygon.h>
#include <io/poly_reader.h>
#include <io/off_writer.h>
#include <triangulation/triangle_triangulator.h>
#include <triangulation/acute_triangulator.h>

int main() {
    omg::Polygon poly = omg::io::readPoly("apps/medsea.poly");
    std::cout << "input polygon:" << std::endl;
    std::cout << "vertices: " << poly.getVertices().size() << std::endl;
    std::cout << "edges: " << poly.getEdges().size() << std::endl << std::endl;

    omg::Mesh mesh;
    omg::TriangleTriangulator tri;
    tri.generateMesh(poly, mesh);

    std::cout << "output mesh:" << std::endl;
    std::cout << "vertices: " << mesh.n_vertices() << std::endl;
    std::cout << "triangles: " << mesh.n_faces() << std::endl;

    omg::io::writeOff("apps/medsea.off", mesh);

    omg::Mesh mesh2;
    omg::ACuteTriangulator tri2;
    tri2.generateMesh(poly, mesh2);

    std::cout << "output mesh:" << std::endl;
    std::cout << "vertices: " << mesh2.n_vertices() << std::endl;
    std::cout << "triangles: " << mesh2.n_faces() << std::endl;

    omg::io::writeOff("apps/medsea2.off", mesh2);
}