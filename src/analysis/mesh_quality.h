#pragma once

#include <unordered_map>

#include <mesh/mesh.h>
#include <size_function/size_function.h>

namespace omg {
namespace analysis {

template<typename T>
struct Aggregates {
    T min, max, avg;

    Aggregates() : min(0), max(0), avg(0) {}

    template<typename Iter>
    Aggregates(Iter begin, Iter end) : Aggregates() {

        const std::size_t count = std::distance(begin, end);
        if (count == 0) {
            throw std::runtime_error("cannot aggregate empty data");
        }

        min = max = static_cast<T>(*begin);

        for (; begin != end; ++begin) {
            const T v = static_cast<T>(*begin);

            min = std::min(min, v);
            max = std::max(max, v);
            avg += v;
        }
        avg /= count;
    }
};

std::vector<real_t> computeRadiusRatio(const Mesh& mesh);

std::vector<std::size_t> countValences(const Mesh& mesh);
std::unordered_map<std::size_t, std::size_t> groupValences(const Mesh& mesh);
void printValences(const Mesh& mesh);

std::vector<int> computeValenceDeviation(const Mesh& mesh);

std::vector<real_t> computeEdgeLength(const Mesh& mesh);

std::vector<real_t> computeRelativeEdgeLength(const Mesh& mesh, const SizeFunction& size, std::size_t samples = 10);

}
}
