#include <iostream>

#include <omg.h>

int main() {
    omg::ScopeTimer timer("Total");

    const std::string DIR = "../../apps/data/";

    //omg::LineGraph poly = omg::io::readPoly(DIR + "medsea.poly");
    omg::LineGraph poly = omg::LineGraph::createRectangle({{20, -60}, {130, 40}});
    omg::io::writeLegacyVTK(DIR + "poly.vtk", poly);

    omg::BathymetryData topo = omg::io::readNetCDF(DIR + "GEBCO_2020.nc", poly.computeBoundingBox());
    omg::io::writeLegacyVTK(DIR + "bathymetry.vtk", topo);

    omg::Resolution resolution;
    resolution.coarsest = 20000;
    resolution.finest = 1000;
    resolution.coastal = 5000;
    //resolution.aois.push_back({omg::vec2_t(25.14, 35.335), 0.2, 0.5, 1000});  // Heraklion

    omg::ReferenceSize sf(topo, resolution);

    class TestSize : public omg::SizeFunction {
    public:
        TestSize() : omg::SizeFunction({{-1, -1}, {1, 1}}, {101, 101}) {
            std::fill(grid().begin(), grid().end(), 1);
            grid(50, 50) = 0;
        }
    };
    //TestSize sf;

    //omg::io::writeLegacyVTK(DIR + "size_fkt.vtk", sf, true);

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
    omg::io::writeLegacyVTK(DIR + "coast.vtk", omg::LineGraph(coast.getOuter()));

    if (coast.hasIntersections()) {
        std::cout << "boundary intersection" << std::endl;
    }

    omg::Mesh mesh;
    omg::TriangleTriangulator tri;
    // omg::JigsawTriangulator tri;
    tri.generateMesh(coast, sf, mesh);

    std::cout << "output mesh:" << std::endl;
    std::cout << "vertices: " << mesh.n_vertices() << std::endl;
    std::cout << "triangles: " << mesh.n_faces() << std::endl;

    //omg::io::writeLegacyVTK(DIR + "mesh.vtk", mesh);

    omg::IsotropicRemeshing ir(sf);
    ir.remesh(mesh, 200);

    omg::io::writeLegacyVTK(DIR + "remesh.vtk", mesh);
}
