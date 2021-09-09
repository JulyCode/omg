
#include "mesh_quality.h"

#include <iostream>
#include <math.h>

namespace omg {
namespace analysis {

std::vector<real_t> computeQuality(const Mesh& mesh) {
    // computes 2 * incircle radius / circumcircle redius
    std::vector<real_t> quality;

    for (const auto& fh : mesh.faces()) {

        // vertices
        const vec2_t& va = toVec2(mesh.point(fh.halfedge().from()));
        const vec2_t& vb = toVec2(mesh.point(fh.halfedge().to()));
        const vec2_t& vc = toVec2(mesh.point(fh.halfedge().next().to()));

        // edge lengths
        const real_t a = (vb - vc).norm();
        const real_t b = (va - vc).norm();
        const real_t c = (va - vb).norm();

        // incircle radius
        // see https://en.wikipedia.org/wiki/Incircle_and_excircles_of_a_triangle
        const real_t s = (a + b + c) / 2;
        const real_t in_radius = std::sqrt((s - a) * (s - b) * (s - c) / s);

        // circumcircle radius
        // see https://en.wikipedia.org/wiki/Circumscribed_circle
        const real_t cos_a = ((vb - va) / c).dot((vc - va) / b);
        const real_t sin_a = std::sqrt(1 - cos_a * cos_a);
        const real_t out_radius = 0.5 * a / sin_a;

        if (std::isnan(in_radius)) {
            std::cout << "nan" << std::endl;
            continue;
        }
        if (sin_a == 0) {
            std::cout << "sin" << std::endl;
            continue;
        }
        if (!std::isfinite(out_radius)) {
            std::cout << "inf" << std::endl;
            continue;
        }

        quality.push_back(2 * in_radius / out_radius);
    }

    return quality;
}

QualityStats computeQualityStats(const Mesh& mesh) {
    const std::vector<real_t> quality = computeQuality(mesh);
    if (quality.size() == 0) {
        throw std::runtime_error("empty quality list");
    }

    QualityStats stats = {quality[0], quality[0], 0};

    for (const real_t q : quality) {
        stats.min = std::min(stats.min, q);
        stats.max = std::max(stats.max, q);
        stats.avg += q;
    }
    stats.avg /= quality.size();

    return stats;
}

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
