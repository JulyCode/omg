#pragma once

#include <boundary/boundary.h>
#include <mesh/mesh.h>

namespace omg {

// save wrapper for triangulateio and triangleio

template<typename io_t>
class TriangleIn {
public:
    explicit TriangleIn(const Boundary& boundary);
    ~TriangleIn();

    io_t io;
};

template<typename io_t>
class TriangleOut {
public:
    TriangleOut();
    ~TriangleOut();

    void toMesh(Mesh& mesh) const;

    io_t io;
};

}
