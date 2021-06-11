#pragma once

#include <geometry/polygon.h>

namespace omg {

class Triangulator {
public:
    virtual void generateMesh(const Polygon& outline) = 0;
};

}
