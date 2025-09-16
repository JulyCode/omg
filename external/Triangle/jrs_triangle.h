#pragma once

namespace jrs {  // wrap Triangle in a C++ namespace to avoid conflicts

#ifndef ANSI_DECLARATORS
#define ANSI_DECLARATORS
#endif
#define REAL double
#define VOID void
#ifndef EXTERNAL_TEST
#define EXTERNAL_TEST
#endif
#ifndef NO_TIMER
#define NO_TIMER
#endif

#define TRILIBRARY

#include "triangle/triangle.h"

#include <functional>

void set_triunsuitable_callback(std::function<int(REAL*, REAL*, REAL*, REAL)> callback);

}

