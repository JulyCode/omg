#pragma once

#include <geometry/he_polygon.h>
#include <geometry/line_graph.h>
#include <size_function/size_function.h>

namespace omg {

void simplifyPolygon(HEPolygon& poly, const SizeFunction& size, real_t min_angle_deg);

// invalidates all handles!
void removeDegeneratedGeometry(LineGraph& graph);

}
