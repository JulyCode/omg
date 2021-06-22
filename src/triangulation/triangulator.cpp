
#include "triangulator.h"

#include <geometry/types.h>

namespace omg {

void Triangulator::restrictToInt(const Polygon& outline) const {
    const std::size_t num_vertices = outline.getVertices().size();
    const std::size_t num_edges = outline.getEdges().size();

    // throw error if sizes exceed ints
    if (!fitsInt(num_vertices)) {
        throw std::runtime_error("Too many vertices in outline");
    }
    if (!fitsInt(num_edges)) {
        throw std::runtime_error("Too many edges in outline");
    }
}

}
