#pragma once

#include <unordered_map>

#include <mesh/mesh.h>
#include <size_function/size_function.h>

namespace omg {
namespace analysis {

std::vector<real_t> computeRadiusRatio(const Mesh& mesh);

std::vector<std::size_t> countValences(const Mesh& mesh);
std::unordered_map<std::size_t, std::size_t> groupValences(const Mesh& mesh);
void printValences(const Mesh& mesh);

std::vector<int> computeValenceDeviation(const Mesh& mesh);

std::vector<real_t> computeEdgeLength(const Mesh& mesh);

std::vector<real_t> computeRelativeEdgeLength(const Mesh& mesh, const SizeFunction& size, std::size_t samples = 10);

}
}
