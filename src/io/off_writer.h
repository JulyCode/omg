#pragma once

#include <string>

#include <geometry/mesh.h>

namespace omg {
namespace io {

void writeOff(const std::string& filename, const Mesh& mesh);  // TODO: improve IO system

}
}
