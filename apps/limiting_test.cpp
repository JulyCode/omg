#include <iostream>

#include <omg.h>

int main() {
    omg::ScopeTimer timer("Mesh generation");

    const std::string DIR = "../../apps/data/";

    const omg::AxisAlignedBoundingBox gibraltar = {{-6.23, 35.68}, {-5.12, 36.29}};
    const omg::AxisAlignedBoundingBox crete = {{22.86, 34.29}, {26.63, 36.07}};
    omg::LineGraph poly_box = omg::LineGraph::createRectangle(crete);

    const omg::LineGraph& poly = poly_box;

    omg::BathymetryData topo = omg::io::readNetCDF(DIR + "GEBCO_2020.nc", poly.computeBoundingBox());
    omg::io::writeLegacyVTK(DIR + "bathymetry.vtk", topo);

    omg::Resolution resolution;
    resolution.coarsest = 600000;
    resolution.finest = 2000;
    resolution.coastal = 15000;
    //resolution.aois.push_back({omg::vec2_t(-5.63, 35.93), 0.17, 0.19, 100});  // Gibraltar
    resolution.aois.push_back({omg::vec2_t(25.14, 35.335), 0.6, 0.7, 2000});  // Heraklion

    std::vector<double> limits = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0};

    omg::io::CSVTable table;

    omg::ReferenceSize sf_orig(topo, resolution);
    omg::fastGradientLimiting(sf_orig, 1.0);

    //omg::Boundary coast(topo, poly, sf_orig);
    //coast.generate();

    //if (coast.hasIntersections()) {
    //    std::cout << "warning: boundary intersection" << std::endl;
    //}

    for (std::size_t i = 0; i < limits.size(); i++) {
        omg::ReferenceSize sf(topo, resolution);
        omg::fastGradientLimiting(sf, limits[i]);

        omg::Boundary coast(topo, poly, sf);
        coast.generate();

        if (coast.hasIntersections()) {
            std::cout << "warning: boundary intersection" << std::endl;
        }

        omg::Mesh mesh;
        omg::TriangleTriangulator tri;
        tri.generateMesh(coast, sf, mesh);

        omg::IsotropicRemeshing ir(sf);
        ir.remesh(mesh, 200);

        std::vector<omg::real_t> shape_reg = omg::analysis::computeShapeRegularity(mesh);
        //omg::analysis::Aggregates<omg::real_t> quality_sr(shape_reg.begin(), shape_reg.end());
        //std::cout << quality_sr.min << ", " << quality_sr.max << ", " << quality_sr.avg << std::endl;
        table.addColumn(std::to_string(i), shape_reg.begin(), shape_reg.end());

        if (i == 0 || i == limits.size() - 1) {
            omg::io::writeLegacyVTK(DIR + "limit_mesh" + std::to_string(limits[i]) + ".vtk", mesh);
        }
    }

    table.write(DIR + "limit_stats.csv");
}
