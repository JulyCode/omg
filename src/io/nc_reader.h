#pragma once

#include <topology/scalar_field.h>

namespace omg {
namespace io {

ScalarField<int16_t> read_netCDF(const std::string& filename, const AxisAlignedBoundingBox& aabb);

ScalarField<int16_t> read_netCDF(const std::string& filename);

}
}
