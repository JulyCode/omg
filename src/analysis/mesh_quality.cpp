
#include "mesh_quality.h"

#include <iostream>

namespace omg {
namespace analysis {

std::unordered_map<std::size_t, std::size_t> countValences(const Mesh& mesh) {
    std::unordered_map<std::size_t, std::size_t> histogram;

    for (const auto& vh : mesh.vertices()) {
        const int valence = mesh.valence(vh);
        if (histogram.find(valence) == histogram.end()) {
            histogram[valence] = 0;
        }
        histogram[valence]++;
    }

    return histogram;
}

void printValences(const Mesh& mesh) {
    const auto& valences = omg::analysis::countValences(mesh);

    std::vector<std::pair<std::size_t, std::size_t>> sorted;
    for (const auto& p : valences) {
        sorted.push_back(p);
    }

    std::sort(sorted.begin(), sorted.end(), [](const auto& p1, const auto& p2) { return p1.first < p2.first; });

    for (const auto& p : sorted) {
        std::cout << "valence " << p.first << ": " << p.second << std::endl;
    }
}

}
}
