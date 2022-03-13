#include <iostream>

#include <omg.h>

int main() {
    omg::ScopeTimer timer("Mesh generation");

    const std::string DIR = "../../../apps/data/";

    const omg::LineGraph poly = omg::io::readPoly(DIR + "medsea.poly");

    omg::BathymetryData topo = omg::io::readNetCDF(DIR + "GEBCO_2020.nc", poly.computeBoundingBox());
    // omg::io::writeLegacyVTK(DIR + "bathymetry.vtk", topo);

    omg::Resolution resolution_medsea;
    resolution_medsea.coarsest = 20000;
    resolution_medsea.finest = 1000;
    resolution_medsea.coastal = 5000;
    resolution_medsea.aois.push_back({omg::vec2_t(25.14, 35.335), 0.2, 0.5, 1000});  // Heraklion

    omg::ReferenceSize sf(topo, resolution_medsea);

    //omg::io::writeLegacyVTK(DIR + "size_fkt.vtk", sf, true);

    //omg::fastGradientLimiting(sf, 0.3);

    //omg::io::writeLegacyVTK(DIR + "size_fkt_limited.vtk", sf, true);

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

    omg::io::writeLegacyVTK(DIR + "tri_mesh.vtk", mesh);

    omg::IsotropicRemeshing ir(sf);
    ir.remesh(mesh, 20);

    omg::io::writeLegacyVTK(DIR + "remesh.vtk", mesh);

    mesh = omg::Mesh();
    jig.generateMesh(coast, sf, mesh);

    omg::io::writeLegacyVTK(DIR + "jig_mesh.vtk", mesh);
}
