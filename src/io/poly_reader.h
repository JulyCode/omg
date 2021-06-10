#pragma once

#include <string>

#include <geometry/polygon.h>

namespace omg
{
namespace io
{

Polygon readPoly(const std::string& filename);

}
}
