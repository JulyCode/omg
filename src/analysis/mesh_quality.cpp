
#include "mesh_quality.h"

#include <iostream>
#include <math.h>

#include <mesh/remeshing.h>

namespace omg {
namespace analysis {

std::vector<real_t> computeRadiusRatio(const Mesh& mesh) {
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

std::vector<std::size_t> countValences(const Mesh& mesh) {
    std::vector<std::size_t> valences;

    for (const auto& vh : mesh.vertices()) {
        valences.push_back(mesh.valence(vh));
    }

    return valences;
}

std::unordered_map<std::size_t, std::size_t> groupValences(const Mesh& mesh) {
    std::unordered_map<std::size_t, std::size_t> histogram;

    for (std::size_t valence : countValences(mesh)) {

        if (histogram.find(valence) == histogram.end()) {
            histogram[valence] = 0;
        }
        histogram[valence]++;
    }

    return histogram;
}

void printValences(const Mesh& mesh) {
    const auto& valences = omg::analysis::groupValences(mesh);

    std::vector<std::pair<std::size_t, std::size_t>> sorted;
    for (const auto& p : valences) {
        sorted.push_back(p);
    }

    std::sort(sorted.begin(), sorted.end(), [](const auto& p1, const auto& p2) { return p1.first < p2.first; });

    for (const auto& p : sorted) {
        std::cout << "valence " << p.first << ": " << p.second << std::endl;
    }
}

std::vector<int> computeValenceDeviation(const Mesh& mesh) {
    std::vector<int> deviation;

    for (const auto& vh : mesh.vertices()) {
        const int valence = mesh.valence(vh);

        const int optimal = IsotropicRemeshing::computeOptimalValence(vh, mesh);

        deviation.push_back(valence - optimal);
    }

    return deviation;
}

std::vector<real_t> computeEdgeLength(const Mesh& mesh) {
    std::vector<real_t> length;

    for (const auto& heh : mesh.halfedges()) {

        length.push_back((mesh.point(heh.from()) - mesh.point(heh.to())).norm());
    }

    return length;
}

std::vector<real_t> computeRelativeEdgeLength(const Mesh& mesh, const SizeFunction& size, std::size_t samples) {
    std::vector<real_t> rel_length;

    if (samples == 0) {
        throw std::runtime_error("zero samples");
    }

    for (const auto& heh : mesh.halfedges()) {

        const vec2_t& p0 = toVec2(mesh.point(heh.from()));
        const vec2_t& p1 = toVec2(mesh.point(heh.to()));

        const real_t length = (p1 - p0).norm();

        real_t target_size = 0;

        if (samples > 1) {
            // average size function samples over edge
            for (std::size_t i = 0; i < samples; i++) {

                const real_t lambda = i / (samples - 1);
                const vec2_t p = lambda * p0 + (1 - lambda) * p1;

                target_size += size.getValue(p);
            }

            target_size /= samples;
        } else {
            // use center
            target_size = size.getValue((p0 + p1) / 2);
        }

        rel_length.push_back(length / target_size);
    }

    return rel_length;
}

}
}
