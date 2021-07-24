#pragma once

#include <topology/scalar_field.h>
#include <geometry/line_graph.h>

namespace omg {

LineGraph marchingQuads(const BathymetryData& data, real_t iso_value);

}
