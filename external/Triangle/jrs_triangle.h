#pragma once

namespace jrs {  // wrap Triangle in a C++ namespace to avoid conflicts

#ifndef ANSI_DECLARATORS
#define ANSI_DECLARATORS
#endif
#define REAL double
#define VOID void

#include "triangle.h"

void set_triunsuitable_callback(int (*callback) (REAL* v1, REAL* v2, REAL* v3, REAL area));

}
