#pragma once

#include <topology/scalar_field.h>
#include <geometry/line_graph.h>
#include <geometry/mesh.h>

namespace omg {
namespace io {

template<typename T>
void writeLegacyVTK(const std::string& filename, const ScalarField<T>& data, bool binary = false);

void writeLegacyVTK(const std::string& filename, const LineGraph& poly);

void writeLegacyVTK(const std::string& filename, const Mesh& mesh);

}
}
