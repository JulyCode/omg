#pragma once

#include <unordered_map>

#include <mesh/mesh.h>

namespace omg {
namespace analysis {

struct QualityStats {
    real_t min, max, avg;
};

std::vector<real_t> computeQuality(const Mesh& mesh);
QualityStats computeQualityStats(const Mesh& mesh);

std::unordered_map<std::size_t, std::size_t> countValences(const Mesh& mesh);
void printValences(const Mesh& mesh);

}
}
