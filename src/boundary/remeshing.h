#pragma once

#include <geometry/he_polygon.h>
#include <size_function/size_function.h>

namespace omg {

void remesh(HEPolygon& poly, const SizeFunction& size);

}
