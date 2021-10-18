#pragma once

#include <size_function/size_function.h>

namespace omg {

void simpleGradientLimiting(SizeFunction& size, real_t limit, real_t time_step, std::size_t iterations = 200);

void fastGradientLimiting(SizeFunction& size, real_t limit);

void jigsawGradientLimiting(SizeFunction& size, real_t limit);

}
