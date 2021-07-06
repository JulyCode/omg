#include <iostream>

#include <geometry/polygon.h>
#include <io/poly_reader.h>
#include <io/off_writer.h>
#include <io/nc_reader.h>
#include <triangulation/triangle_triangulator.h>
#include <triangulation/acute_triangulator.h>
#include <size_function/constant_size.h>

int main() {

    omg::AxisAlignedBoundingBox aabb;
    // aabb.min = omg::vec2_t(29.0020833333334, 41.0020833333334);
    // aabb.max = omg::vec2_t(29.0979166666666, 41.0979166666666);
    aabb.min = omg::vec2_t(-8.1, 28.1);
    aabb.max = omg::vec2_t(41.0, 47.9);

    omg::ScalarField<int16_t> topo = omg::io::read_netCDF("../../apps/GEBCO_2020.nc", aabb);
    std::cout << topo.getBoundingBox().min << std::endl;
    std::cout << topo.getBoundingBox().max << std::endl;

    // std::cout << topo.getValue<omg::real_t>(omg::vec2_t(29.0020833333334, 41.0020833333334)) << std::endl;  // should be -20
    // std::cout << topo.getValue<omg::real_t>(omg::vec2_t(29.0979166666666, 41.0979166666666)) << std::endl;  // should be 115
    // std::cout << topo.getValue<omg::real_t>(omg::vec2_t(29.00625, 41.00625)) << std::endl;  // should be -14
    // std::cout << topo.getValue<omg::real_t>(omg::vec2_t(29.09375, 41.09375)) << std::endl;  // should be 123
    // std::cout << topo.getValue<omg::real_t>(omg::vec2_t(29.0020833333334, 41.01875)) << std::endl;  // should be -18
    // std::cout << topo.getValue<omg::real_t>(omg::vec2_t(29.0979166666666, 41.08125)) << std::endl;  // should be 100
    // std::cout << topo.getValue<omg::real_t>(omg::vec2_t(29.0040833333334, 41.0040833333334)) << std::endl;  // should be between -20 and  -15

    omg::ConstantSize sf(topo.getBoundingBox(), 0.1);

    omg::Polygon poly = omg::io::readPoly("../../apps/medsea.poly");
    std::cout << "input polygon:" << std::endl;
    std::cout << "vertices: " << poly.getVertices().size() << std::endl;
    std::cout << "edges: " << poly.getEdges().size() << std::endl << std::endl;

    omg::Mesh mesh;
    omg::TriangleTriangulator tri;
    tri.generateMesh(poly, sf, mesh);

    std::cout << "output mesh:" << std::endl;
    std::cout << "vertices: " << mesh.n_vertices() << std::endl;
    std::cout << "triangles: " << mesh.n_faces() << std::endl;

    omg::io::writeOff("../../apps/medsea.off", mesh);

    omg::Mesh mesh2;
    omg::ACuteTriangulator tri2;
    tri2.generateMesh(poly, sf, mesh2);

    std::cout << "output mesh:" << std::endl;
    std::cout << "vertices: " << mesh2.n_vertices() << std::endl;
    std::cout << "triangles: " << mesh2.n_faces() << std::endl;

    omg::io::writeOff("../../apps/medsea2.off", mesh2);
}