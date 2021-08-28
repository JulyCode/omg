#pragma once

#include <unordered_map>

#include <mesh/mesh.h>

namespace omg {
namespace analysis {

std::unordered_map<std::size_t, std::size_t> countValences(const Mesh& mesh);
void printValences(const Mesh& mesh);

}
}
