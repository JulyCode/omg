#include <iostream>

#include <omg.h>

int main() {
    omg::ScopeTimer timer("Mesh generation");

    const std::string DIR = "../../../apps/data/";

    const omg::AxisAlignedBoundingBox indian_ocean = {{20, -60}, {130, 40}};
    const omg::AxisAlignedBoundingBox baltic_sea = {{9, 53.5}, {31, 66}};
    const omg::AxisAlignedBoundingBox black_sea = {{27, 40}, {42, 47.5}};
    const omg::AxisAlignedBoundingBox madagascar = {{42, -27}, {51.5, -9}};
    const omg::AxisAlignedBoundingBox north_atlantic_ocean = {{-81, 5}, {0, 58}};
    const omg::AxisAlignedBoundingBox lake_constance = {{8.83, 47.39}, {9.82, 47.92}};  // height 404
    const omg::AxisAlignedBoundingBox bosporus = {{28.83, 40.96}, {29.21, 41.25}};
    const omg::AxisAlignedBoundingBox crete = {{22.86, 34.29}, {26.63, 36.07}};
    const omg::AxisAlignedBoundingBox nile = {{31.82, 25.99}, {32.31, 26.41}};  // res 4100, height 66
    omg::LineGraph poly_box = omg::LineGraph::createRectangle(indian_ocean);

    const omg::LineGraph poly_medsea = omg::io::readPoly(DIR + "medsea.poly");
    const omg::LineGraph poly_south_baltic(omg::HEPolygon(
        {{11.7, 54}, {14.5, 53.8}, {17.2, 54.5}, {16.5, 55.5}, {14.6, 56.2}, {12, 55.6}}));

    const omg::LineGraph& poly = poly_medsea;
    // omg::io::writeLegacyVTK(DIR + "poly.vtk", poly);

    omg::BathymetryData topo = omg::io::readNetCDF(DIR + "GEBCO_2020.nc", poly.computeBoundingBox());
    // omg::io::writeLegacyVTK(DIR + "bathymetry.vtk", topo);

    omg::Resolution resolution_medsea;
    resolution_medsea.coarsest = 20000;
    resolution_medsea.finest = 1000;
    resolution_medsea.coastal = 5000;
    resolution_medsea.aois.push_back({omg::vec2_t(25.14, 35.335), 0.2, 0.5, 1000});  // Heraklion

    omg::Resolution resolution_indian;
    resolution_indian.coarsest = 200000;
    resolution_indian.finest = 2000;
    resolution_indian.coastal = 40000;
    resolution_indian.aois.push_back({omg::vec2_t(96.03, 4.92), 2.0, 3.0, 2000});  // Aceh

    omg::Resolution resolution_north_atlantic;
    resolution_north_atlantic.coarsest = 100000;
    resolution_north_atlantic.finest = 10000;
    resolution_north_atlantic.coastal = 35000;
    resolution_north_atlantic.aois.push_back({omg::vec2_t(-5.63, 35.93), 1.5, 3.0, 10000});  // Gibraltar

    omg::Resolution resolution_south_baltic;
    resolution_south_baltic.coarsest = 180000;
    resolution_south_baltic.finest = 1500;
    resolution_south_baltic.coastal = 6000;
    resolution_south_baltic.aois.push_back({omg::vec2_t(14.7, 55.1), 0.05, 0.2, 1500});

    omg::Resolution resolution_black_sea;
    resolution_black_sea.coarsest = 100000;
    resolution_black_sea.finest = 1500;
    resolution_black_sea.coastal = 8000;

    omg::Resolution resolution_lake_constance;
    resolution_lake_constance.coarsest = 20000;
    resolution_lake_constance.finest = 100;
    resolution_lake_constance.coastal = 1200;

    omg::Resolution resolution_bosporus;
    resolution_bosporus.coarsest = 20000;
    resolution_bosporus.finest = 100;
    resolution_bosporus.coastal = 3000;

    omg::ReferenceSize sf(topo, resolution_medsea);

    //omg::io::writeLegacyVTK(DIR + "size_fkt.vtk", sf, true);

    //omg::fastGradientLimiting(sf, 0.3);

    //omg::io::writeLegacyVTK(DIR + "size_fkt_limited.vtk", sf, true);

    /*std::vector<omg::real_t> grad_norm = omg::analysis::computeGradientNorm(sf, omg::analysis::Norm::EUCLIDEAN);
    omg::analysis::Aggregates<omg::real_t> grad(grad_norm.begin(), grad_norm.end());
    std::cout << grad.min << ", " << grad.max << ", " << grad.avg << std::endl;

    grad_norm = omg::analysis::computeGradientNorm(sf, omg::analysis::Norm::MAXIMUM);
    grad = omg::analysis::Aggregates<omg::real_t>(grad_norm.begin(), grad_norm.end());
    std::cout << grad.min << ", " << grad.max << ", " << grad.avg << std::endl;*/

    //omg::io::writeLegacyVTK(DIR + "size_limited.vtk", sf, true);
    //return 0;

    omg::Boundary coast(topo, poly, sf);
    coast.generate(100, false, true, 120);
    //omg::io::writeLegacyVTK(DIR + "outer.vtk", omg::LineGraph(coast.getOuter()));

    if (coast.hasIntersections()) {
        std::cout << "warning: boundary intersection" << std::endl;
    }

    omg::Mesh mesh;
    omg::TriangleTriangulator tri;
    omg::JigsawTriangulator jig;
    tri.generateMesh(coast, sf, mesh);

    std::cout << "output mesh:" << std::endl;
    std::cout << "vertices: " << mesh.n_vertices() << std::endl;
    std::cout << "triangles: " << mesh.n_faces() << std::endl;

    omg::io::writeLegacyVTK(DIR + "tri_mesh.vtk", mesh);
    omg::io::writeNod2D(mesh, topo);

    omg::IsotropicRemeshing ir(sf);
    ir.remesh(mesh, 20);

    omg::io::writeLegacyVTK(DIR + "remesh.vtk", mesh);
    omg::io::writeNod2D(mesh, topo, DIR + "remesh");

    mesh = omg::Mesh();
    jig.generateMesh(coast, sf, mesh);

    omg::io::writeLegacyVTK(DIR + "jig_mesh.vtk", mesh);
}
