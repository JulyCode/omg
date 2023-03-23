
#ifdef OMG_REQUIRE_NETCDF

#include "nc_reader.h"

#include <netcdf>
#include <functional>

namespace omg {
namespace io {

static const AxisAlignedBoundingBox EVERYTHING;
static const real_t MAX_LON_COORD = 180;
static const real_t MIN_LON_COORD = -180;

static bool startsWith(const std::string& str, const std::string& expr) {
    if (str.size() < expr.size()) {
        return false;
    }
    return str.substr(0, expr.size()) == expr;
}

static netCDF::NcVar getVar(const netCDF::NcFile& data_file, const std::string& var_name) {

    // check if variable is found
    const netCDF::NcVar var = data_file.getVar(var_name);

    if (var.isNull()) {
        throw std::runtime_error("No variable \"" + var_name + "\" found");
    }
    return var;
}

static netCDF::NcVar getAndCheckVar(const netCDF::NcFile& data_file, const std::string& var_name,
                                    const std::string& expected_unit, const netCDF::NcType& expected_type,
                                    std::size_t expected_dim_count) {

    const netCDF::NcVar var = getVar(data_file, var_name);

    // check if the unit is as expected
    const netCDF::NcVarAtt attribute = var.getAtt("units");
    if (attribute.isNull()) {
        throw std::runtime_error("Variable \"" + var_name + "\" has no unit");
    }

    std::string unit;
    attribute.getValues(unit);
    if (unit != expected_unit) {
        throw std::runtime_error("Variable \"" + var_name + "\" has the wrong unit, expected: "
                                + expected_unit + ", found: " + unit);
    }

    // check if the data type is as expected
    if (var.getType() != expected_type) {
        throw std::runtime_error("Variable \"" + var_name + "\" has the wrong type, expected: "
                                + expected_type.getName() + ", found: " + var.getType().getName());
    }

    // check if the dimension count is as expected
    if (static_cast<std::size_t>(var.getDimCount()) != expected_dim_count) {
        throw std::runtime_error("Variable \"" + var_name + "\" has the wrong dimension, expected: "
                                + std::to_string(expected_dim_count) + ", found: " + std::to_string(var.getDimCount()));
    }

    return var;
}


// struct to unify coordinate access
struct Coordinate {

    Coordinate(const netCDF::NcVar var, real_t (*getFromVar)(const netCDF::NcVar& var, std::size_t idx))
        : var(var), getFromVar(getFromVar) {}

    const netCDF::NcVar var;

    real_t get(std::size_t idx) const {
        return getFromVar(var, idx);
    }

    real_t (*getFromVar)(const netCDF::NcVar& var, std::size_t idx);
};

struct DataHandle {

    DataHandle(const netCDF::NcVar ele, Coordinate lon, Coordinate lat, const AxisAlignedBoundingBox& aabb)
        : elevation(ele), longitude(lon), latitude(lat), aabb(aabb) {}

    const netCDF::NcVar elevation;

    Coordinate longitude;
    Coordinate latitude;

    const AxisAlignedBoundingBox& aabb;
};

static std::size_t getClosestIndexBelow(const Coordinate& data, real_t coordinate) {
    // search for the highest index with a coordinate lower than the given one
    // assumes coordinates are ordered ascending (according to http://cfconventions.org)

    if (data.var.getDimCount() != 1) {
        throw std::runtime_error("Only one-dimensional data is allowed");
    }
    const std::size_t dim = data.var.getDim(0).getSize();

    const real_t c0 = data.get(0);
    const real_t c1 = data.get(dim - 1);

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

    real_t v = data.get(idx);

    // increase until above
    while (v < coordinate) {
        idx++;

        if (idx >= dim) {
            // should never happen
            throw std::runtime_error("Coordinate is not included in the dataset");
        }

        v = data.get(idx);
    }

    // decrease until below
    while (v > coordinate) {
        idx--;

        if (idx >= dim) {
            // rollover should never happen
            throw std::runtime_error("Coordinate is not included in the dataset");
        }

        v = data.get(idx);
    }

    return idx;
}

static void getIndicesToRead(size2_t& from_idx, size2_t& to_idx, const DataHandle& data) {

    // check dimensions
    const netCDF::NcDim lat_dim = data.latitude.var.getDim(0);
    const netCDF::NcDim lon_dim = data.longitude.var.getDim(0);
    const netCDF::NcDim ele_dim0 = data.elevation.getDim(0);
    const netCDF::NcDim ele_dim1 = data.elevation.getDim(1);

    if (lat_dim != ele_dim0 || lon_dim != ele_dim1) {
        throw std::runtime_error("Dimensions of coordinates and elevation data don't match");
    }

    // section of the data to be read
    from_idx = size2_t(0);
    to_idx = size2_t(lon_dim.getSize(), lat_dim.getSize());  // exclusive

    if (to_idx[0] < 2 || to_idx[1] < 2) {
        throw std::runtime_error("The elevation grid has to have at least 2x2 values");
    }

    // only read the part according to aabb
    if (&data.aabb != &EVERYTHING) {
        if (data.aabb.min[0] != MIN_LON_COORD) {
            from_idx[0] = getClosestIndexBelow(data.longitude, data.aabb.min[0]);
        }
        from_idx[1] = getClosestIndexBelow(data.latitude, data.aabb.min[1]);

        if (data.aabb.max[0] != MAX_LON_COORD) {
            to_idx[0] = getClosestIndexBelow(data.longitude, data.aabb.max[0]) + 2;  // + 2 to get above and exclusive
        }
        to_idx[1] = getClosestIndexBelow(data.latitude, data.aabb.max[1]) + 2;
    }
}


static BathymetryData readGEBCO20(const netCDF::NcFile& data_file, const AxisAlignedBoundingBox& aabb) {

    // latitude coordinates for the sample points of the elevation grid
    const netCDF::NcVar latitude = getAndCheckVar(data_file, "lat", "degrees_north", netCDF::ncDouble, 1);

    // longitude coordinates for the sample points of the elevation grid
    const netCDF::NcVar longitude = getAndCheckVar(data_file, "lon", "degrees_east", netCDF::ncDouble, 1);

    // 2D elevation grid with height in meters as 16 bit signed int
    const netCDF::NcVar elevation = getAndCheckVar(data_file, "elevation", "m", netCDF::ncShort, 2);

    // set up data handle
    auto readCoord = [](const netCDF::NcVar& var, std::size_t idx) {
        real_t value;
        var.getVar({idx}, &value);
        return value;
    };

    Coordinate lat(latitude, readCoord);
    Coordinate lon(longitude, readCoord);
    DataHandle handle(elevation, lon, lat, aabb);

    size2_t from_idx, to_idx;
    getIndicesToRead(from_idx, to_idx, handle);

    const size2_t grid_size = to_idx - from_idx;

    // read boundary coordinates (assumes coordinates are ordered ascending)
    AxisAlignedBoundingBox real_aabb;  // the actual bb of the data read may be a little bit larger than requested
    real_aabb.min = vec2_t(readCoord(longitude, from_idx[0]), readCoord(latitude, from_idx[1]));
    real_aabb.max = vec2_t(readCoord(longitude, to_idx[0] - 1), readCoord(latitude, to_idx[1] - 1));

    BathymetryData topo(real_aabb, grid_size);

    // read topography data
    const std::vector<std::size_t> start = {from_idx[1], from_idx[0]};
    const std::vector<std::size_t> count = {to_idx[1] - from_idx[1], to_idx[0] - from_idx[0]};
    elevation.getVar(start, count, topo.grid().data());

    return topo;
}

static BathymetryData readGEBCO08(const netCDF::NcFile& data_file, const AxisAlignedBoundingBox& aabb) {

    // latitude coordinates for the sample points of the elevation grid
    const netCDF::NcVar latitude = getVar(data_file, "lat");

    // longitude coordinates for the sample points of the elevation grid
    const netCDF::NcVar longitude = getVar(data_file, "lon");

    // 2D elevation grid with height in meters as float
    const netCDF::NcVar elevation = getVar(data_file, "topo");

    // set up data handle
    auto readLat = [](const netCDF::NcVar& var, std::size_t idx) {
        real_t value;
        var.getVar({idx, 1}, &value);
        return value;
    };

    auto readLon = [](const netCDF::NcVar& var, std::size_t idx) {
        real_t value;
        var.getVar({idx, 0}, &value);
        return value;
    };

    Coordinate lat(latitude, readLat);
    Coordinate lon(longitude, readLon);
    DataHandle handle(elevation, lon, lat, aabb);

    size2_t from_idx, to_idx;
    getIndicesToRead(from_idx, to_idx, handle);

    const size2_t grid_size = to_idx - from_idx;

    // read boundary coordinates (assumes coordinates are ordered ascending)
    AxisAlignedBoundingBox real_aabb;  // the actual bb of the data read may be a little bit larger than requested
    real_aabb.min = vec2_t(readLon(longitude, from_idx[0]), readLat(latitude, from_idx[1]));
    real_aabb.max = vec2_t(readLon(longitude, to_idx[0] - 1), readLat(latitude, to_idx[1] - 1));

    BathymetryData topo(real_aabb, grid_size);

    // read topography data
    const std::vector<std::size_t> start = {from_idx[1], from_idx[0]};
    const std::vector<std::size_t> count = {to_idx[1] - from_idx[1], to_idx[0] - from_idx[0]};

    // buffer needed to convert from float to int16_t
    std::vector<int16_t>& grid = topo.grid();
    float* buffer = new float[grid.size()];
    elevation.getVar(start, count, buffer);

    for (std::size_t i = 0; i < grid.size(); i++) {
        grid[i] = static_cast<int16_t>(buffer[i]);
        assert(static_cast<float>(grid[i]) == buffer[i]);
    }

    delete[] buffer;

    return topo;
}


static void concatBathymetry(BathymetryData& dst, const BathymetryData& low, const BathymetryData& high) {
    const std::size_t low_stride = low.getGridSize()[0];
    const std::size_t high_stride = high.getGridSize()[0];

    const std::size_t rows = dst.getGridSize()[1];
    auto low_it = low.grid().begin();
    auto high_it = high.grid().begin();
    auto dst_it = dst.grid().begin();

    for (std::size_t y = 0; y < rows; y++) {
        dst_it = std::copy(low_it, low_it + low_stride, dst_it);
        low_it += low_stride;
        dst_it = std::copy(high_it, high_it + high_stride, dst_it);
        high_it += high_stride;
    }
}


BathymetryData readNetCDF(const std::string& filename) {
    return readNetCDF(filename, EVERYTHING);
}

BathymetryData readNetCDF(const std::string& filename, const AxisAlignedBoundingBox& aabb) {
    try {
        // open netcdf file
        const netCDF::NcFile data_file(filename, netCDF::NcFile::read);

        // check if the file is supported
        const netCDF::NcGroupAtt attribute = data_file.getAtt("title");
        if (attribute.isNull()) {
            throw std::runtime_error("No title was found");
        }

        std::string title;
        attribute.getValues(title);

        // select reader to use
        std::function<BathymetryData(const netCDF::NcFile&, const AxisAlignedBoundingBox&)> reader;
        if (startsWith(title, "The GEBCO_2020 Grid")) {
            reader = readGEBCO20;
        } else if (startsWith(title, "GEBCO_08 TOPOGRAPHY")) {
            reader = readGEBCO08;
        } else {
            throw std::runtime_error("This file format is not supported");
        }

        // check if (-180, 180) or (0, 360) is used for lon
        if (&aabb == &EVERYTHING || aabb.max[0] < MAX_LON_COORD) {
            return reader(data_file, aabb);
        }
        if (aabb.min[0] > MAX_LON_COORD && aabb.max[0] > MAX_LON_COORD) {
            AxisAlignedBoundingBox mod_aabb = aabb;
            mod_aabb.min[0] = mod_aabb.min[0] - 2 * MAX_LON_COORD;
            mod_aabb.max[0] = mod_aabb.max[0] - 2 * MAX_LON_COORD;

            BathymetryData mod_data = reader(data_file, mod_aabb);
            mod_aabb = mod_data.getBoundingBox();
            mod_aabb.min[0] = mod_aabb.min[0] + 2 * MAX_LON_COORD;
            mod_aabb.max[0] = mod_aabb.max[0] + 2 * MAX_LON_COORD;

            BathymetryData data(mod_aabb, mod_data.getGridSize());
            data.grid() = std::move(mod_data.grid());
            return data;
        }
        if (aabb.min[0] < MAX_LON_COORD && aabb.max[0] > MAX_LON_COORD) {
            AxisAlignedBoundingBox mod_aabb = aabb;
            mod_aabb.max[0] = MAX_LON_COORD;
            const BathymetryData low_data = reader(data_file, mod_aabb);

            mod_aabb = aabb;
            mod_aabb.min[0] = MIN_LON_COORD;
            mod_aabb.max[0] = mod_aabb.max[0] - 2 * MAX_LON_COORD;
            const BathymetryData high_data = reader(data_file, mod_aabb);

            const size2_t grid_size(low_data.getGridSize()[0] + high_data.getGridSize()[0], low_data.getGridSize()[1]);
            mod_aabb = low_data.getBoundingBox();
            mod_aabb.max[0] = high_data.getBoundingBox().max[0] + 2 * MAX_LON_COORD;

            BathymetryData data(mod_aabb, grid_size);
            concatBathymetry(data, low_data, high_data);
            return data;
        }
        throw std::runtime_error("Invalid bounding box");

    } catch(netCDF::exceptions::NcException& e) {
        throw std::runtime_error("Error reading data from " + filename + ": " + e.what());
    }
}

}
}

#endif  // OMG_REQUIRE_NETCDF
