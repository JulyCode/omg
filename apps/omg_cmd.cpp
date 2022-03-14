
#include <iostream>
#include <fstream>
#include <iomanip>

#include <omg.h>

#include <nlohmann/json.hpp>

enum class PolyType {
    FILE,
    RECTANGLE,
    INVALID = -1
};

NLOHMANN_JSON_SERIALIZE_ENUM(PolyType, {
    {PolyType::INVALID, nullptr},
    {PolyType::FILE, "file"},
    {PolyType::RECTANGLE, "rectangle"}
})


enum class LimitingMethod {
    NONE,
    OMG,
    MARCHE,
    INVALID = -1
};

NLOHMANN_JSON_SERIALIZE_ENUM(LimitingMethod, {
    {LimitingMethod::INVALID, nullptr},
    {LimitingMethod::NONE, "none"},
    {LimitingMethod::OMG, "omg"},
    {LimitingMethod::MARCHE, "marche"}
})


enum class Triangulator {
    TRIANGLE,
    JIGSAW,
    INVALID = -1
};

NLOHMANN_JSON_SERIALIZE_ENUM(Triangulator, {
    {Triangulator::INVALID, nullptr},
    {Triangulator::TRIANGLE, "triangle"},
    {Triangulator::JIGSAW, "jigsaw"}
})


enum class FileFormat {
    VTK,
    OFF,
    NOD2D,
    INVALID = -1
};

NLOHMANN_JSON_SERIALIZE_ENUM(FileFormat, {
    {FileFormat::INVALID, nullptr},
    {FileFormat::VTK, "vtk"},
    {FileFormat::OFF, "off"},
    {FileFormat::NOD2D, "nod2d"}
})


int main(int argc, char* args[]) {
    omg::ScopeTimer timer;
    std::cout << "========== Ocean Mesh Generation ==========" << std::endl;

    // TODO: change to cmd args
    // if (argc < 2) {
    //     std::cout << "Path to a config.json file required!" << std::endl;
    //     return EXIT_SUCCESS;
    // }
    // const std::string filename = args[1];
    const std::string filename = "../../apps/config.json";

    std::cout << "Reading configuration file ..." << std::endl;
    nlohmann::json cfg;

    std::ifstream cfg_file(filename);
    if (!cfg_file.good()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    cfg_file >> cfg;

    std::cout << "Creating region polygon ..." << std::endl;
    omg::LineGraph poly;

    const PolyType poly_type = cfg["poly_region"]["type"].get<PolyType>();
    switch (poly_type) {
        case PolyType::FILE:
            poly = omg::io::readPoly(cfg["poly_region"]["path"].get<std::string>());
            break;
        case PolyType::RECTANGLE:
            {
                omg::AxisAlignedBoundingBox rect;
                const auto& min = cfg["poly_region"]["min"];
                rect.min = {min[0].get<omg::real_t>(), min[1].get<omg::real_t>()};
                const auto& max = cfg["poly_region"]["max"];
                rect.max = {max[0].get<omg::real_t>(), max[1].get<omg::real_t>()};

                poly = omg::LineGraph::createRectangle(rect);
            }
            break;
        default:
            std::cerr << "Invalid region type!" << std::endl;
            return EXIT_FAILURE;
    }

    std::cout << "Reading bathymetry data ..." << std::endl;
    const std::string nc_filename = cfg["netcdf_bathymetry"].get<std::string>();
    const omg::BathymetryData topo = omg::io::readNetCDF(nc_filename, poly.computeBoundingBox());

    omg::real_t coast_height = 0;
    if (cfg.contains("sea_level")) {
        std::cout << "Setting relative sea level ..." << std::endl;
        coast_height = cfg["sea_level"].get<omg::real_t>();
    }

    std::cout << "Setting resolution ..." << std::endl;
    omg::Resolution resolution;
    resolution.coarsest = cfg["resolution"]["coarsest"].get<omg::real_t>();
    resolution.finest = cfg["resolution"]["finest"].get<omg::real_t>();
    resolution.coastal = cfg["resolution"]["coastal"].get<omg::real_t>();

    if (cfg["resolution"].contains("aoi")) {
        for (auto& j : cfg["resolution"]["aoi"]) {

            const omg::vec2_t center(j["center_pos"][0].get<omg::real_t>(), j["center_pos"][1].get<omg::real_t>());
            const omg::real_t inner = j["inner_radius"].get<omg::real_t>();
            const omg::real_t outer = j["outer_radius"].get<omg::real_t>();
            const omg::real_t res = j["element_size"].get<omg::real_t>();

            const omg::AreaOfInterest aoi(center, inner, outer, res);
            resolution.aois.push_back(aoi);
        }
    }

    std::cout << "Creating size function ..." << std::endl;
    omg::ReferenceSize sf(topo, resolution);

    if (cfg.contains("gradient_limiting")) {

        const LimitingMethod method = cfg["gradient_limiting"]["method"].get<LimitingMethod>();
        if (method != LimitingMethod::NONE) {
            std::cout << "Applying gradient limiting ..." << std::endl;

            switch (method) {
                case LimitingMethod::OMG:
                    omg::fastGradientLimiting(sf, cfg["gradient_limiting"]["limit"]);
                    break;
                case LimitingMethod::MARCHE:
                    omg::jigsawGradientLimiting(sf, cfg["gradient_limiting"]["limit"]);
                    break;
                default:
                    std::cerr << "Invalid gradient limiting method!" << std::endl;
                    return EXIT_FAILURE;
            }
        }
    }

    std::cout << "Creating boundary ..." << std::endl;
    omg::Boundary coast(topo, poly, sf);
    const omg::real_t height = cfg["boundary"]["height"].get<omg::real_t>();
    bool ignore_islands = false;
    omg::real_t min_angle = 60;

    if (cfg["boundary"].contains("ignore_islands")) {
        ignore_islands = cfg["boundary"]["ignore_islands"].get<bool>();
    }
    if (cfg["boundary"].contains("min_angle")) {
        min_angle = cfg["boundary"]["min_angle"].get<omg::real_t>();
    }
    coast.generate(height, ignore_islands, true, min_angle);

    if (coast.hasIntersections()) {

        bool allow_self_intersection = false;
        if (cfg["boundary"].contains("allow_self_intersection")) {
            allow_self_intersection = cfg["boundary"]["allow_self_intersection"].get<bool>();
        }

        std::cerr << "Boundary has a self-intersection!" << std::endl;
        if (!allow_self_intersection) {
            return EXIT_FAILURE;
        }
    }

    std::cout << "Preparing triangulation ..." << std::endl;
    omg::Mesh mesh;
    std::unique_ptr<omg::Triangulator> tri;

    const Triangulator triangulator_type = cfg["triangulator"].get<Triangulator>();
    switch (triangulator_type) {
        case Triangulator::TRIANGLE:
            tri = std::make_unique<omg::TriangleTriangulator>();
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

    if (cfg.contains("remeshing_iterations") && triangulator_type == Triangulator::TRIANGLE) {
        const int iterations = cfg["remeshing_iterations"].get<int>();
        if (iterations > 0) {

            std::cout << "Performing remeshing ..." << std::endl;
            omg::IsotropicRemeshing ir(sf);
            ir.remesh(mesh, iterations);
        }
    }

    std::cout << "Saving mesh ..." << std::endl;
    std::string mesh_filename = "";
    if (cfg["output"].contains("mesh_file_path")) {
        mesh_filename = cfg["output"]["mesh_file_path"].get<std::string>();
    }

    const FileFormat format = cfg["output"]["mesh_file_format"].get<FileFormat>();
    switch (format) {
        case FileFormat::VTK:
            omg::io::writeLegacyVTK(mesh_filename.empty() ? "out.vtk" : mesh_filename, mesh);
            break;
        case FileFormat::OFF:
            omg::io::writeOff(mesh_filename.empty() ? "out.off" : mesh_filename, mesh);
            break;
        case FileFormat::NOD2D:
            omg::io::writeNod2D(mesh, topo, mesh_filename);
            break;
        default:
            std::cerr << "Invalid file format!" << std::endl;
            return EXIT_FAILURE;
    }

    if (cfg["output"].contains("save_bathymetry")) {
        const std::string file = cfg["output"]["save_bathymetry"].get<std::string>();
        if (!file.empty()) {
            std::cout << "Saving bathymetry ..." << std::endl;
            omg::io::writeLegacyVTK(file, topo);
        }
    }
    if (cfg["output"].contains("save_size_function")) {
        const std::string file = cfg["output"]["save_size_function"].get<std::string>();
        if (!file.empty()) {
            std::cout << "Saving size function ..." << std::endl;
            omg::io::writeLegacyVTK(file, sf);
        }
    }
    if (cfg["output"].contains("save_boundary")) {
        const std::string file = cfg["output"]["save_boundary"].get<std::string>();
        if (!file.empty()) {
            std::cout << "Saving boundary ..." << std::endl;

            std::vector<omg::HEPolygon> polys = coast.getIslands();
            polys.push_back(coast.getOuter());

            const omg::LineGraph complete = omg::LineGraph::combinePolygons(polys);

            omg::io::writeLegacyVTK(file, complete);
        }
    }
}
