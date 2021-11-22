#include <iostream>

#include <omg.h>

void addDataToCSV(omg::io::CSVTable& table, const std::string& name, const omg::Mesh& mesh, const omg::SizeFunction& sf) {

    std::vector<int> valence_dev = omg::analysis::computeValenceDeviation(mesh);
    omg::analysis::Aggregates<omg::real_t> vd(valence_dev.begin(), valence_dev.end());
    std::cout << vd.min << ", " << vd.max << ", " << vd.avg << std::endl;
    table.addColumn(name + " valence", valence_dev.begin(), valence_dev.end());

    std::vector<omg::real_t> radius_ratio = omg::analysis::computeRadiusRatio(mesh);
    omg::analysis::Aggregates<omg::real_t> quality(radius_ratio.begin(), radius_ratio.end());
    std::cout << quality.min << ", " << quality.max << ", " << quality.avg << std::endl;
    table.addColumn(name + " radius ratio", radius_ratio.begin(), radius_ratio.end());

    std::vector<omg::real_t> edge_length = omg::analysis::computeRelativeEdgeLength(mesh, sf, 10);
    omg::analysis::Aggregates<omg::real_t> el(edge_length.begin(), edge_length.end());
    std::cout << el.min << ", " << el.max << ", " << el.avg << std::endl;
    table.addColumn(name + " edge length", edge_length.begin(), edge_length.end());
}

int main() {
    omg::ScopeTimer timer("Total");

    const std::string DIR = "../../apps/data/";

    omg::LineGraph poly = omg::io::readPoly(DIR + "medsea.poly");
    // omg::io::writeLegacyVTK(DIR + "poly.vtk", poly);

    omg::BathymetryData topo = omg::io::readNetCDF(DIR + "GEBCO_2020.nc", poly.computeBoundingBox());
    // omg::io::writeLegacyVTK(DIR + "bathymetry.vtk", topo);

    omg::Resolution resolution;
    resolution.coarsest = 20000;
    resolution.finest = 1000;
    resolution.coastal = 5000;
    resolution.aois.push_back({omg::vec2_t(25.14, 35.335), 0.2, 0.5, 1000});  // Heraklion

    omg::ReferenceSize sf(topo, resolution);

    // omg::io::writeLegacyVTK(DIR + "size_fkt.vtk", sf, true);

    //omg::fastGradientLimiting(sf, 0.3);

    /*std::vector<omg::real_t> grad_norm = omg::analysis::computeGradientNorm(sf, omg::analysis::Norm::EUCLIDEAN);
    omg::analysis::Aggregates<omg::real_t> grad(grad_norm.begin(), grad_norm.end());
    std::cout << grad.min << ", " << grad.max << ", " << grad.avg << std::endl;

    grad_norm = omg::analysis::computeGradientNorm(sf, omg::analysis::Norm::MAXIMUM);
    grad = omg::analysis::Aggregates<omg::real_t>(grad_norm.begin(), grad_norm.end());
    std::cout << grad.min << ", " << grad.max << ", " << grad.avg << std::endl;*/

    //omg::io::writeLegacyVTK(DIR + "size_limited.vtk", sf, true);
    //return 0;

    omg::Boundary coast(topo, poly, sf);
    coast.generate();
    // omg::io::writeLegacyVTK(DIR + "coast.vtk", omg::LineGraph(coast.getOuter()));

    if (coast.hasIntersections()) {
        std::cout << "boundary intersection" << std::endl;
    }

    omg::Mesh tri_mesh, jig_mesh;
    omg::TriangleTriangulator tri;
    omg::JigsawTriangulator jig;
    tri.generateMesh(coast, sf, tri_mesh);
    jig.generateMesh(coast, sf, jig_mesh);
    
    omg::io::CSVTable table;
    addDataToCSV(table, "tri", tri_mesh, sf);
    addDataToCSV(table, "jig", jig_mesh, sf);

    //omg::io::writeLegacyVTK(DIR + "mesh.vtk", mesh);

    omg::IsotropicRemeshing ir(sf);
    ir.remesh(tri_mesh, 200);

    addDataToCSV(table, "tri remesh", tri_mesh, sf);

    table.write(DIR + "stats.csv");

    //omg::io::writeLegacyVTK(DIR + "remesh.vtk", mesh);
}
