
#include "size_gradient.h"

#include <mutex>

namespace omg {
namespace analysis {

std::vector<real_t> computeGradientNorm(const SizeFunction& size, Norm norm) {
    std::vector<real_t> grad;
    std::mutex mutex;

    #pragma omp parallel for
    for (std::size_t i = 0; i < size.getGridSize()[0]; i++) {
        for (std::size_t j = 0; j < size.getGridSize()[1]; j++) {

            const vec2_t g = size.computeGradient({i, j});
            real_t length;

            switch (norm) {
                case Norm::EUCLIDEAN:
                    length = g.norm();
                    break;
                case Norm::MAXIMUM:
                    length = std::max(std::abs(g[0]), std::abs(g[1]));
                    break;
            }

            const std::lock_guard lock(mutex);
            grad.push_back(length);
        }
    }

    return grad;
}

}
}
