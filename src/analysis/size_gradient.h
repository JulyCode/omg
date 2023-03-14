#pragma once

#include <size_function/size_function.h>

namespace omg {
namespace analysis {

enum class Norm {
    EUCLIDEAN, MAXIMUM
};

std::vector<real_t> computeGradientNorm(const SizeFunction& size, Norm norm);

}
}
