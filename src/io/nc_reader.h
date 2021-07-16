#pragma once

#include <topology/scalar_field.h>

namespace omg {
namespace io {

BathymetryData readNetCDF(const std::string& filename, const AxisAlignedBoundingBox& aabb);

BathymetryData readNetCDF(const std::string& filename);

}
}
