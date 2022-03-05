#pragma once

#include <string>

#include <mesh/mesh.h>
#include <topology/scalar_field.h>

namespace omg {
namespace io {

void writeNod2D(const Mesh& mesh, const BathymetryData& topo, const std::string& name = "", bool zero_based = false);

}
}
