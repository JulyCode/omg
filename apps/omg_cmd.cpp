
#include <iostream>
#include <fstream>
#include <iomanip>

#include <util.h>
#include <io/poly_reader.h>
#include <io/nc_reader.h>
#include <size_function/reference_size.h>
#include <triangulation/triangle_triangulator.h>
#include <triangulation/acute_triangulator.h>
#include <triangulation/jigsaw_triangulator.h>
#include <mesh/remeshing.h>
#include <io/vtk_writer.h>
#include <io/off_writer.h>

#include <nlohmann/json.hpp>

enum class Triangulator {
    TRIANGLE,
    ACUTE,
    JIGSAW,
    INVALID = -1
};

NLOHMANN_JSON_SERIALIZE_ENUM(Triangulator, {
    {Triangulator::INVALID, nullptr},
    {Triangulator::TRIANGLE, "triangle"},
    {Triangulator::ACUTE, "acute"},
    {Triangulator::JIGSAW, "jigsaw"}
})

enum class FileFormat {
    VTK,
    OFF,
    INVALID = -1
};

NLOHMANN_JSON_SERIALIZE_ENUM(FileFormat, {
    {FileFormat::INVALID, nullptr},
    {FileFormat::VTK, "vtk"},
    {FileFormat::OFF, "off"}
})

int main() {
    omg::ScopeTimer timer("Done! Total duration");

    std::cout << "Ocean Mesh Generation" << std::endl;

    const std::string filename = "../../apps/cfg.json";

    std::cout << "Reading configuration file ..." << std::endl;
    nlohmann::json cfg;

    std::ifstream cfg_file(filename);
    if (!cfg_file.good()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    cfg_file >> cfg;

    std::cout << "Creating region polygon ..." << std::endl;
    const omg::LineGraph poly = omg::io::readPoly(cfg["poly_region"].get<std::string>());

    std::cout << "Reading bathymetry data ..." << std::endl;
    const std::string nc_filename = cfg["netcdf_bathymetry"].get<std::string>();
    const omg::BathymetryData topo = omg::io::readNetCDF(nc_filename, poly.computeBoundingBox());

    std::cout << "Setting resolution ..." << std::endl;
    omg::Resolution resolution;
    resolution.coarsest = cfg["resolution"]["coarsest"].get<omg::real_t>();
    resolution.finest = cfg["resolution"]["finest"].get<omg::real_t>();
    resolution.coastal = cfg["resolution"]["coastal"].get<omg::real_t>();

    for (auto& j : cfg["resolution"]["aois"]) {

        const omg::vec2_t center(j["center_pos"][0].get<omg::real_t>(), j["center_pos"][1].get<omg::real_t>());
        const omg::real_t inner = j["inner_radius"].get<omg::real_t>();
        const omg::real_t outer = j["outer_radius"].get<omg::real_t>();
        const omg::real_t res = j["resolution"].get<omg::real_t>();

        const omg::AreaOfInterest aoi(center, inner, outer, res);
        resolution.aois.push_back(aoi);
    }

    std::cout << "Creating size function ..." << std::endl;
    const omg::ReferenceSize sf(topo, resolution);

    std::cout << "Extracting boundary ..." << std::endl;
    omg::Boundary coast(topo, poly, sf);
    coast.generate(cfg["sea level"].get<omg::real_t>());

    if (coast.hasIntersections()) {
        std::cerr << "Boundary has a self-intersection!" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Preparing triangulation ..." << std::endl;
    omg::Mesh mesh;
    std::unique_ptr<omg::Triangulator> tri;

    const Triangulator type = cfg["triangulator"].get<Triangulator>();
    switch (type) {
        case Triangulator::TRIANGLE:
            tri = std::make_unique<omg::TriangleTriangulator>();
            break;
        case Triangulator::ACUTE:
            tri = std::make_unique<omg::ACuteTriangulator>();
            break;
        case Triangulator::JIGSAW:
            tri = std::make_unique<omg::JigsawTriangulator>();
            break;
        default:
            std::cerr << "Invalid triangulator!" << std::endl;
            return EXIT_FAILURE;
    }

    std::cout << "Constructing mesh ..." << std::endl;
    tri->generateMesh(coast, sf, mesh);

    const int iterations = cfg["remeshing_iterations"].get<int>();
    if (iterations > 0) {

        std::cout << "Performing remeshing ..." << std::endl;
        omg::IsotropicRemeshing ir(sf);
        ir.remesh(mesh, iterations);
    }

    std::cout << "Saving mesh ..." << std::endl;
    const std::string mesh_filename = cfg["mesh_destination"].get<std::string>();

    const FileFormat format = cfg["mesh_file_format"].get<FileFormat>();
    switch (format) {
        case FileFormat::VTK:
            omg::io::writeLegacyVTK(mesh_filename, mesh);
            break;
        case FileFormat::OFF:
            omg::io::writeOff(mesh_filename, mesh);
            break;
        default:
            std::cerr << "Invalid file format!" << std::endl;
            return EXIT_FAILURE;
    }
}
