#pragma once

#include <topology/scalar_field.h>

namespace omg {
namespace io {

BathymetryData readBin32Topology(const std::string& lon_file, const std::string& lat_file, const std::string& topo_file,
                                 const size2_t& grid_size);

ScalarField<real_t> readBin32Gradient(const std::string& lon_file, const std::string& lat_file,
                                      const std::string& grad_file, const size2_t& grid_size);

}
}
