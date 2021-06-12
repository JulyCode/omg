
#include "triangulator.h"

namespace omg {

void Triangulator::restrictToInt(const Polygon& outline) const {
    const std::size_t num_vertices = outline.getVertices().size();
    const std::size_t num_edges = outline.getEdges().size();

    // throw error if sizes exceed 32 bits
    if (num_vertices == 0 || num_vertices >= std::numeric_limits<int>::max()) {
        throw std::runtime_error("Invalid number of vertices in outline");
    }
    if (num_edges == 0 || num_edges >= std::numeric_limits<int>::max()) {
        throw std::runtime_error("Invalid number of edges in outline");
    }
}

}
