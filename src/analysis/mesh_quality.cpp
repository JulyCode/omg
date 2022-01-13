
#include "mesh_quality.h"

#include <iostream>
#include <math.h>

#include <mesh/remeshing.h>
#include <util.h>

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

        const real_t cos_a = std::clamp((va - vb).dot(vc - vb) / (a * c), -1.0, 1.0);
        const real_t cos_b = std::clamp((vb - va).dot(vc - va) / (b * c), -1.0, 1.0);
        const real_t alpha = std::acos(cos_a);
        const real_t beta = std::acos(cos_b);

        const real_t sin_a = std::sin(alpha);
        const real_t sin_b = std::sin(beta);
        const real_t sin_ab = std::sin(alpha + beta);

        if (alpha == 0 || alpha == PI || beta == 0 || beta == PI) {
            quality.push_back(0);
        } else {
            const real_t rr = (sin_a + sin_b + sin_ab) / (2 * sin_a * sin_b * sin_ab);
            quality.push_back(2 / rr);
        }
    }

    return quality;
}

std::vector<real_t> computeShapeRegularity(const Mesh& mesh) {
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

        const real_t l_rms_sqr = a * a + b * b + c * c;
        const real_t signed_area = toVec3(vb - va).cross(toVec3(vc - va))[2] / 2;

        quality.push_back(4 * std::sqrt(3) * signed_area / l_rms_sqr);
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

    for (const auto& eh : mesh.edges()) {

        length.push_back((mesh.point(eh.v0()) - mesh.point(eh.v1())).norm());
    }

    return length;
}

std::vector<real_t> computeRelativeEdgeLength(const Mesh& mesh, const SizeFunction& size, std::size_t samples) {
    std::vector<real_t> rel_length;

    if (samples == 0) {
        throw std::runtime_error("zero samples");
    }

    for (const auto& eh : mesh.edges()) {

        const vec2_t& p0 = toVec2(mesh.point(eh.v0()));
        const vec2_t& p1 = toVec2(mesh.point(eh.v1()));

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
