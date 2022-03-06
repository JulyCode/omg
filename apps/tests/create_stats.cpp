#include <iostream>

#include <omg.h>

const std::string DIR = "../../../apps/data/";

void addDataToCSV(omg::io::CSVTable& table, const std::string& name, const omg::Mesh& mesh, const omg::SizeFunction& sf) {

    std::vector<int> valence_dev = omg::analysis::computeValenceDeviation(mesh);
    omg::analysis::Aggregates<omg::real_t> vd(valence_dev.begin(), valence_dev.end());
    std::cout << vd.min << ", " << vd.max << ", " << vd.avg << std::endl;
    table.addColumn(name + " valence", valence_dev.begin(), valence_dev.end());

    std::vector<omg::real_t> radius_ratio = omg::analysis::computeRadiusRatio(mesh);
    omg::analysis::Aggregates<omg::real_t> quality_rr(radius_ratio.begin(), radius_ratio.end());
    std::cout << quality_rr.min << ", " << quality_rr.max << ", " << quality_rr.avg << std::endl;
    table.addColumn(name + " radius ratio", radius_ratio.begin(), radius_ratio.end());

    std::vector<omg::real_t> shape_reg = omg::analysis::computeShapeRegularity(mesh);
    omg::analysis::Aggregates<omg::real_t> quality_sr(shape_reg.begin(), shape_reg.end());
    std::cout << quality_sr.min << ", " << quality_sr.max << ", " << quality_sr.avg << std::endl;
    table.addColumn(name + " shape regularity", shape_reg.begin(), shape_reg.end());

    std::vector<omg::real_t> edge_length = omg::analysis::computeRelativeEdgeLength(mesh, sf, 10);
    omg::analysis::Aggregates<omg::real_t> el(edge_length.begin(), edge_length.end());
    std::cout << el.min << ", " << el.max << ", " << el.avg << std::endl;
    table.addColumn(name + " edge length", edge_length.begin(), edge_length.end());
}

void triangulate(omg::io::CSVTable& table, const omg::Boundary& coast, const omg::SizeFunction& sf, const std::string& option) {
    omg::Mesh tri_mesh, jig_mesh;
    omg::TriangleTriangulator tri;
    omg::JigsawTriangulator jig;
    tri.generateMesh(coast, sf, tri_mesh);
    jig.generateMesh(coast, sf, jig_mesh);

    addDataToCSV(table, "tri" + option, tri_mesh, sf);
    addDataToCSV(table, "jig" + option, jig_mesh, sf);

    omg::io::writeLegacyVTK(DIR + "tri" + option + ".vtk", tri_mesh);
    omg::io::writeLegacyVTK(DIR + "jig" + option + ".vtk", jig_mesh);

    omg::IsotropicRemeshing ir(sf);

    ir.remesh(tri_mesh, 200);
    addDataToCSV(table, "tri remesh" + option, tri_mesh, sf);

    ir.remesh(tri_mesh, 200);
    addDataToCSV(table, "jig remesh" + option, jig_mesh, sf);

    omg::io::writeLegacyVTK(DIR + "tri remesh" + option + ".vtk", tri_mesh);
    omg::io::writeLegacyVTK(DIR + "jig remesh" + option + ".vtk", jig_mesh);
}

int main() {
    omg::ScopeTimer timer("Total");

    //omg::LineGraph poly = omg::io::readPoly(DIR + "medsea.poly");
    //const omg::AxisAlignedBoundingBox crete = {{22.86, 34.29}, {26.63, 36.07}};
    const omg::AxisAlignedBoundingBox north_atlantic_ocean = {{-81, 5}, {0, 58}};
    omg::LineGraph poly = omg::LineGraph::createRectangle(north_atlantic_ocean);
    // omg::io::writeLegacyVTK(DIR + "poly.vtk", poly);

    omg::BathymetryData topo = omg::io::readNetCDF(DIR + "GEBCO_2020.nc", poly.computeBoundingBox());
    // omg::io::writeLegacyVTK(DIR + "bathymetry.vtk", topo);

    omg::Resolution resolution;
    resolution.coarsest = 20000;
    resolution.finest = 1000;
    resolution.coastal = 5000;
    resolution.aois.push_back({omg::vec2_t(25.14, 35.335), 0.2, 0.5, 1000});  // Heraklion

    omg::Resolution resolution_north_atlantic;
    resolution_north_atlantic.coarsest = 100000;
    resolution_north_atlantic.finest = 10000;
    resolution_north_atlantic.coastal = 35000;
    resolution_north_atlantic.aois.push_back({omg::vec2_t(-5.63, 35.93), 1.5, 3.0, 10000});  // Gibraltar

    omg::io::CSVTable table;

    {
        omg::ReferenceSize sf(topo, resolution_north_atlantic);

        omg::Boundary coast(topo, poly, sf);
        coast.generate();
        if (coast.hasIntersections()) {
            std::cout << "boundary intersection" << std::endl;
        }

        triangulate(table, coast, sf, "");
    }

    {
        omg::ReferenceSize sf_limited(topo, resolution_north_atlantic);
        omg::fastGradientLimiting(sf_limited, 0.2);

        omg::Boundary coast(topo, poly, sf_limited);
        coast.generate();
        if (coast.hasIntersections()) {
            std::cout << "boundary intersection" << std::endl;
        }

        triangulate(table, coast, sf_limited, " limited");
    }

    {
        omg::ReferenceSize sf_marche(topo, resolution_north_atlantic);
        omg::jigsawGradientLimiting(sf_marche, 0.2);

        omg::Boundary coast(topo, poly, sf_marche);
        coast.generate();
        if (coast.hasIntersections()) {
            std::cout << "boundary intersection" << std::endl;
        }

        triangulate(table, coast, sf_marche, " marche");
    }

    /*std::vector<omg::real_t> grad_norm = omg::analysis::computeGradientNorm(sf, omg::analysis::Norm::EUCLIDEAN);
    omg::analysis::Aggregates<omg::real_t> grad(grad_norm.begin(), grad_norm.end());
    std::cout << grad.min << ", " << grad.max << ", " << grad.avg << std::endl;

    grad_norm = omg::analysis::computeGradientNorm(sf, omg::analysis::Norm::MAXIMUM);
    grad = omg::analysis::Aggregates<omg::real_t>(grad_norm.begin(), grad_norm.end());
    std::cout << grad.min << ", " << grad.max << ", " << grad.avg << std::endl;*/

    // omg::io::writeLegacyVTK(DIR + "size_limited.vtk", sf, true);

    table.write(DIR + "stats.csv");
}
