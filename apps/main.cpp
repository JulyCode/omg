#include <iostream>

#include <geometry/line_graph.h>
#include <io/poly_reader.h>
#include <io/off_writer.h>
#include <io/vtk_writer.h>
#include <io/nc_reader.h>
#include <io/csv_writer.h>
#include <triangulation/triangle_triangulator.h>
#include <triangulation/jigsaw_triangulator.h>
#include <size_function/reference_size.h>
#include <boundary/boundary.h>
#include <mesh/remeshing.h>
#include <analysis/mesh_quality.h>
#include <util.h>

int main() {
    omg::ScopeTimer timer("Total");

    omg::AxisAlignedBoundingBox aabb;
    aabb.min = omg::vec2_t(-20.99583243, 10.99583245);
    aabb.max = omg::vec2_t(47.00416564, 51.99583436);

    omg::LineGraph poly = omg::io::readPoly("../../apps/medsea.poly");

    omg::BathymetryData topo = omg::io::readNetCDF("../../apps/GEBCO_2020.nc", poly.computeBoundingBox());
    std::cout << topo.getGridSize() << std::endl;

    omg::Resolution resolution;
    resolution.coarsest = 20000;
    resolution.finest = 1000;
    resolution.coastal = 5000;
    resolution.aois.push_back({omg::vec2_t(25.14, 35.335), 0.2, 0.5, 1000});  // Heraklion

    omg::ReferenceSize sf(topo, resolution);

    // omg::io::writeLegacyVTK("../../apps/size_fkt.vtk", sf, true);

    omg::Boundary coast(topo, poly, sf);
    coast.generate();

    if (coast.hasIntersections()) {
        std::cout << "boundary intersection" << std::endl;
    }

    omg::io::writeLegacyVTK("../../apps/outer.vtk", omg::LineGraph(coast.getOuter()));

    // omg::io::writeLegacyVTK("../../apps/coast.vtk", coast);

    // return 0;

    omg::Mesh mesh;
    omg::TriangleTriangulator tri;
    //omg::JigsawTriangulator tri;
    tri.generateMesh(coast, sf, mesh);

    std::cout << "output mesh:" << std::endl;
    std::cout << "vertices: " << mesh.n_vertices() << std::endl;
    std::cout << "triangles: " << mesh.n_faces() << std::endl;

    omg::io::writeOff("../../apps/medsea.off", mesh);
    omg::io::writeLegacyVTK("../../apps/mesh.vtk", mesh);

    omg::analysis::printValences(mesh);

    omg::io::CSVTable before;

    std::vector<int> valence_dev = omg::analysis::computeValenceDeviation(mesh);
    omg::analysis::Aggregates<omg::real_t> vd(valence_dev.begin(), valence_dev.end());
    std::cout << vd.min << ", " << vd.max << ", " << vd.avg << std::endl;
    before.addColumn("Valence", valence_dev.begin(), valence_dev.end());

    std::vector<omg::real_t> radius_ratio = omg::analysis::computeRadiusRatio(mesh);
    omg::analysis::Aggregates<omg::real_t> quality(radius_ratio.begin(), radius_ratio.end());
    std::cout << quality.min << ", " << quality.max << ", " << quality.avg << std::endl;
    before.addColumn("Radius ratio", radius_ratio.begin(), radius_ratio.end());

    std::vector<omg::real_t> edge_length = omg::analysis::computeRelativeEdgeLength(mesh, sf, 1);
    omg::analysis::Aggregates<omg::real_t> el(edge_length.begin(), edge_length.end());
    std::cout << el.min << ", " << el.max << ", " << el.avg << std::endl;
    before.addColumn("Edge length", edge_length.begin(), edge_length.end());

    omg::IsotropicRemeshing ir(sf);
    ir.remesh(mesh);

    std::cout << "smoothed mesh:" << std::endl;
    std::cout << "vertices: " << mesh.n_vertices() << std::endl;
    std::cout << "triangles: " << mesh.n_faces() << std::endl;

    omg::io::writeOff("../../apps/medsea_remesh.off", mesh);
    omg::io::writeLegacyVTK("../../apps/medsea_remesh.vtk", mesh);

    omg::analysis::printValences(mesh);

    valence_dev = omg::analysis::computeValenceDeviation(mesh);
    vd = omg::analysis::Aggregates<omg::real_t>(valence_dev.begin(), valence_dev.end());
    std::cout << vd.min << ", " << vd.max << ", " << vd.avg << std::endl;

    radius_ratio = omg::analysis::computeRadiusRatio(mesh);
    quality = omg::analysis::Aggregates<omg::real_t>(radius_ratio.begin(), radius_ratio.end());
    std::cout << quality.min << ", " << quality.max << ", " << quality.avg << std::endl;

    edge_length = omg::analysis::computeRelativeEdgeLength(mesh, sf, 1);
    el = omg::analysis::Aggregates<omg::real_t>(edge_length.begin(), edge_length.end());
    std::cout << el.min << ", " << el.max << ", " << el.avg << std::endl;

    before.write("../../apps/stats_before.csv");
}
