
#include "nc_reader.h"

#include <netcdf>

namespace omg {
namespace io {

static const AxisAlignedBoundingBox EVERYTHING;

static netCDF::NcVar getAndCheckVar(const netCDF::NcFile& data_file, const std::string& var_name,
                                    const std::string& expected_unit, const netCDF::NcType& expected_type,
                                    std::size_t expected_dim_count) {

    // check if variable is found
    const netCDF::NcVar var = data_file.getVar(var_name);

    if (var.isNull()) {
        throw std::runtime_error("No variable \"" + var_name + "\" found");
    }

    // check if the unit is as expected
    const netCDF::NcVarAtt attribute = var.getAtt("units");
    if (attribute.isNull()) {
        throw std::runtime_error("Variable \"" + var_name + "\" has no unit");
    }

    std::string unit;
    attribute.getValues(unit);
    if (unit != expected_unit) {
        throw std::runtime_error("Variable \"" + var_name + "\" has the wrong unit, expected: " + expected_unit + ", found: " + unit);
    }

    // check if the data type is as expected
    if (var.getType() != expected_type) {
        throw std::runtime_error("Variable \"" + var_name + "\" has the wrong type, expected: "
                                + expected_type.getName() + ", found: " + var.getType().getName());
    }

    // check if the dimension count is as expected
    if (var.getDimCount() != expected_dim_count) {
        throw std::runtime_error("Variable \"" + var_name + "\" has the wrong dimension, expected: "
                                + std::to_string(expected_dim_count) + ", found: " + std::to_string(var.getDimCount()));
    }

    return var;
}

static real_t readCoord(const netCDF::NcVar& var, std::size_t idx) {
    real_t value;
    var.getVar({idx}, &value);
    return value;
}

static std::size_t getClosestIndexBelow(const netCDF::NcVar& var, real_t coordinate) {
    // search for the highest index with a coordinate lower than the given one
    // assumes coordinates are ordered ascending (according to http://cfconventions.org)

    if (var.getDimCount() != 1) {
        throw std::runtime_error("Only one-dimensional data is allowed");
    }
    const std::size_t dim = var.getDim(0).getSize();
    
    const real_t c0 = readCoord(var, 0);
    const real_t c1 = readCoord(var, dim - 1);

    if (coordinate < c0 || coordinate > c1) {
        throw std::runtime_error("Coordinate is not included in the dataset");
    }

    // compute step size
    const real_t step = (c1 - c0) / dim;

    // estimate search position
    std::size_t idx = (coordinate - c0) / step + 1;
    if (idx >= dim) {
        idx = dim - 1;
    }

    real_t v = readCoord(var, idx);

    // increase until above
    while (v < coordinate) {
        idx++;

        if (idx >= dim) {
            // should never happen
            throw std::runtime_error("Coordinate is not included in the dataset");
        }

        v = readCoord(var, idx);
    }

    // decrease until below
    while (v > coordinate) {
        idx--;

        if (idx >= dim) {
            // rollover should never happen
            throw std::runtime_error("Coordinate is not included in the dataset");
        }

        v = readCoord(var, idx);
    }

    return idx;
}

ScalarField<int16_t> read_netCDF(const std::string& filename) {
    return read_netCDF(filename, EVERYTHING);
}

ScalarField<int16_t> read_netCDF(const std::string& filename, const AxisAlignedBoundingBox& aabb) {
    try {
        // open netcdf file
        const netCDF::NcFile data_file(filename, netCDF::NcFile::read);

        // latitude coordinates for the sample points of the elevation grid
        const netCDF::NcVar latitude = getAndCheckVar(data_file, "lat", "degrees_north", netCDF::ncDouble, 1);
        
        // longitude coordinates for the sample points of the elevation grid
        const netCDF::NcVar longitude = getAndCheckVar(data_file, "lon", "degrees_east", netCDF::ncDouble, 1);

        // 2D elevation grid with height in meters as 16 bit signed int
        const netCDF::NcVar elevation = getAndCheckVar(data_file, "elevation", "m", netCDF::ncShort, 2);

        // check dimensions
        const netCDF::NcDim lat_dim = latitude.getDim(0);
        const netCDF::NcDim lon_dim = longitude.getDim(0);
        const netCDF::NcDim ele_dim0 = elevation.getDim(0);
        const netCDF::NcDim ele_dim1 = elevation.getDim(1);

        if (lat_dim != ele_dim0 || lon_dim != ele_dim1) {
            throw std::runtime_error("Dimensions of coordinates and elevation data don't match");
        }

        // section of the data to be read
        size2_t from_idx(0);
        size2_t to_idx(lat_dim.getSize(), lon_dim.getSize());  // exclusive

        if (to_idx[0] < 2 || to_idx[1] < 2) {
            throw std::runtime_error("The elevation grid has to have at least 2x2 values");
        }

        // only read the part according to aabb
        if (&aabb != &EVERYTHING) {
            from_idx[0] = getClosestIndexBelow(latitude, aabb.min[0]);
            from_idx[1] = getClosestIndexBelow(longitude, aabb.min[1]);

            to_idx[0] = getClosestIndexBelow(latitude, aabb.max[0]) + 2;  // + 2 to get above and exclusive
            to_idx[1] = getClosestIndexBelow(longitude, aabb.max[1]) + 2;
        }

        const size2_t grid_size = to_idx - from_idx;
        std::cout << grid_size[0] << ", " << grid_size[1] << std::endl;

        // read boundary coordinates (assumes coordinates are ordered ascending)
        AxisAlignedBoundingBox real_aabb;  // the actual bb of the data read may be a little bit larger than requested
        real_aabb.min = vec2_t(readCoord(latitude, from_idx[0]), readCoord(longitude, from_idx[1]));
        real_aabb.max = vec2_t(readCoord(latitude, to_idx[0] - 1), readCoord(longitude, to_idx[1] - 1));

        omg::ScalarField<int16_t> topo(real_aabb, grid_size);

        // read topography data
        const std::vector<std::size_t> start = {from_idx[0], from_idx[1]};
        const std::vector<std::size_t> count = {to_idx[0] - from_idx[0], to_idx[1] - from_idx[1]};
        elevation.getVar(start, count, topo.grid().data());

        return topo;

    } catch(netCDF::exceptions::NcException& e) {
        throw std::runtime_error("Error reading data from " + filename + ": " + e.what());
    }
}

}
}
