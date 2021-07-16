
#include "bin32_reader.h"

#include <fstream>

namespace omg {
namespace io {

// dangerous! directly writing and reading binary data to and from files shouldn't be done (without proper formatting)
// this only exists for compatibility with the old solution
template<typename T>
ScalarField<T> readBin32(const std::string& lon_file, const std::string& lat_file, const std::string& topo_file,
                         const size2_t& grid_size) {

    std::ifstream file;
    std::vector<float> buffer;

    AxisAlignedBoundingBox aabb;

    // read longitude and latitude coordinates
    const std::string filename[] = {lon_file, lat_file};
    for (int i = 0; i < 2; i++) {

        file.open(filename[i], std::ios::binary);
        if (!file.good()) {
            throw std::runtime_error("Error reading file: " + filename[i]);
        }

        buffer.resize(grid_size[i]);
        file.read(reinterpret_cast<char*>(buffer.data()), buffer.size() * sizeof(float));

        aabb.min[i] = buffer.front();
        aabb.max[i] = buffer.back();

        file.close();
    }

    // read elevation
    file.open(topo_file, std::ios::binary);
    if (!file.good()) {
        throw std::runtime_error("Error reading file: " + topo_file);
    }

    buffer.resize(grid_size[0] * grid_size[1]);
    file.read(reinterpret_cast<char*>(buffer.data()), buffer.size() * sizeof(float));

    file.close();

    ScalarField<T> topo(aabb, grid_size);

    std::vector<T>& grid = topo.grid();
    // convert to correct type
    for (std::size_t i = 0; i < grid.size(); i++) {
        grid[i] = static_cast<T>(buffer[i]);
        assert(static_cast<float>(grid[i]) == buffer[i]);
    }

    return topo;
}

BathymetryData readBin32Topology(const std::string& lon_file, const std::string& lat_file, const std::string& topo_file,
                                 const size2_t& grid_size) {

    return readBin32<int16_t>(lon_file, lat_file, topo_file, grid_size);
}

ScalarField<real_t> readBin32Gradient(const std::string& lon_file, const std::string& lat_file,
                                      const std::string& grad_file, const size2_t& grid_size) {

    return readBin32<real_t>(lon_file, lat_file, grad_file, grid_size);
}

}
}
