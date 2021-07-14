#include <iostream>

#include <geometry/polygon.h>
#include <io/poly_reader.h>
#include <io/off_writer.h>
#include <io/vtk_writer.h>
#include <io/nc_reader.h>
#include <io/bin32_reader.h>
#include <triangulation/triangle_triangulator.h>
#include <size_function/reference_size.h>
#include <analysis/grid_compare.h>

int main() {

    omg::AxisAlignedBoundingBox aabb;
    aabb.min = omg::vec2_t(-20.99583243, 20.99583245);
    aabb.max = omg::vec2_t(47.00416564, 51.99583436);

    omg::BathymetryData topo_old = omg::io::readNetCDF("../../apps/MedSea.nc");
    omg::BathymetryData topo = omg::io::readNetCDF("../../apps/GEBCO_2020.nc", topo_old.getBoundingBox());

    /*const std::string ref = "../../apps/reference/topo/";
    omg::BathymetryData topo = omg::io::readBin32Topology(ref + "lon_medsea.bin32",
        ref + "lat_medsea.bin32", ref + "topog_medsea.bin32", omg::size2_t(8161, 3721));
    omg::ScalarField<omg::real_t> grad = omg::io::readBin32Gradient(ref + "lon_medsea.bin32",
        ref + "lat_medsea.bin32", ref + "topog_grad_medsea.bin32", omg::size2_t(8161, 3721));
    
    omg::io::writeLegacyVTK("../../apps/topo_old.vtk", topo, true);
    omg::io::writeLegacyVTK("../../apps/topo_grad_old.vtk", grad, true);*/


    /*auto diff1 = omg::analysis::difference<int16_t, omg::real_t>(topo_old, topo);
    omg::io::writeLegacyVTK("../../apps/diff1.vtk", diff1, true);

    auto diff2 = omg::analysis::difference<int16_t, omg::real_t>(topo, topo_old);
    omg::io::writeLegacyVTK("../../apps/diff2.vtk", diff2, true);*/

    omg::Resolution resolution;
    resolution.coarsest = 20000;
    resolution.finest = 1000;
    resolution.coastal = 5000;
    resolution.aois.push_back({omg::vec2_t(25.14, 35.335), 0.2, 0.5, 1000});  // Heraklion

    std::cout << "calculationg size..." << std::endl;
    omg::ReferenceSize sf(topo, resolution);
    std::cout << "calculationg size done" << std::endl;

    // omg::io::writeLegacyVTK("../../apps/size_fkt.vtk", sf, true);

    omg::Polygon poly = omg::io::readPoly("../../apps/medsea.poly");

    omg::Mesh mesh;
    omg::TriangleTriangulator tri;
    tri.generateMesh(poly, sf, mesh);

    std::cout << "output mesh:" << std::endl;
    std::cout << "vertices: " << mesh.n_vertices() << std::endl;
    std::cout << "triangles: " << mesh.n_faces() << std::endl;

    omg::io::writeOff("../../apps/medsea.off", mesh);
}
