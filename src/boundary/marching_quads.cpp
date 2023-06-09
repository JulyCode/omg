
#include "marching_quads.h"

#include <util.h>

#include <iostream>
#include <shared_mutex>
#include <mutex>

namespace omg {

static constexpr unsigned int edge_table[16] = {
    0b0000, 0b1001, 0b0011, 0b1010,
    0b0110, 0b1111, 0b0101, 0b1100,
    0b1100, 0b0101, 0b1111, 0b0110,
    0b1010, 0b0011, 0b1001, 0b0000
};

static vec2_t linearInterpolation(const vec2_t& p1, const vec2_t& p2, real_t v1, real_t v2, real_t iso) {
    real_t factor;

    // don't divide by zero
    if (std::abs(v2 - v1) < 0.00001) {
        factor = 0.5;
    } else {
        factor = (iso - v1) / (v2 - v1);
    }

    vec2_t res = p1 + (p2 - p1) * factor;
    return res;
}

LineGraph marchingQuads(const BathymetryData& data, real_t iso_value) {
    ScopeTimer timer("Marching quads");

    const size2_t& grid_size = data.getGridSize();

    LineGraph poly;

    std::mutex v_lock, e_lock;
    std::shared_mutex map_lock;

    std::unordered_map<std::size_t, std::size_t> point_map;

    #pragma omp parallel for
    for (std::size_t i = 0; i < grid_size[0] - 1; i++) {
        for (std::size_t j = 0; j < grid_size[1] - 1; j++) {

            // compute indices of quad
            std::array<size2_t, 4> idx;
            idx[0] = size2_t(i, j);
            idx[1] = size2_t(i + 1, j);
            idx[2] = size2_t(i + 1 , j + 1);
            idx[3] = size2_t(i, j + 1);

            // read values, compute positions and get lookup index
            std::array<real_t, 4> values;
            std::array<vec2_t, 4> pos;
            unsigned int lookup_index = 0;

            for (int n = 0; n < 4; n++) {
                values[n] = static_cast<real_t>(data.grid(idx[n]));

                pos[n] = data.getPoint(idx[n]);

                // set bit n, if value is below iso
                if (values[n] < iso_value) {
                    lookup_index |= 1 << n;
                }
            }

            const unsigned int edges = edge_table[lookup_index];
            if (edges == 0) {
                continue;  // early exit
            }

            int counter = 0;
            std::array<LineGraph::VertexHandle, 4> points;

            // index to identify edges globally
            const std::size_t edge_base_idx = data.linearIndex(idx[0]) * 2;
            const std::size_t edge_index_offset[4] = { 0, 3, grid_size[0] * 2, 1};

            for (int n = 0; n < 4; n++) {
                // if this edge is used
                if (edges & (1 << n)) {

                    const std::size_t edge_idx = edge_base_idx + edge_index_offset[n];

                    bool found;
                    {
                        const std::shared_lock lock(map_lock);

                        const auto it = point_map.find(edge_idx);
                        found = it != point_map.end();
                        // if this edge already has a point, use that
                        if (found) {
                            points[counter] = it->second;
                        }
                    }

                    if (!found) {  // else

                        // calculate new position for a point on this edge
                        const int m = (n + 1) % 4;  // end point of edge
                        const vec2_t point = linearInterpolation(pos[n], pos[m], values[n], values[m], iso_value);

                        {
                            const std::lock_guard lock(v_lock);
                            points[counter] = poly.addVertex(point);
                        }

                        {
                            const std::unique_lock lock(map_lock);
                            // insert into map
                            point_map[edge_idx] = points[counter];
                        }
                    }

                    counter++;
                }
            }

            // asymptotic decider
            // see http://web.cse.ohio-state.edu/~shen.94/788/Site/Reading_files/p83-nielson.pdf
            if (counter == 4) {
                real_t asymptotic_center_value = values[0] * values[2] + values[1] * values[3];
                asymptotic_center_value /= values[0] + values[2] - values[1] - values[3];

                // swap the points to be connected the other way
                if (asymptotic_center_value < iso_value) {
                    std::swap(points[0], points[2]);
                }
            }

            // connect points to polygon edges
            const std::lock_guard lock(e_lock);

            for (int n = 0; n < counter; n += 2) {
                poly.addEdge(points[n], points[n + 1]);
            }
        }
    }

    return poly;
}

}
