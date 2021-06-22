#pragma once

#include <vector>

#include <geometry/types.h>

namespace omg {

using PolygonEdge = std::pair<std::size_t, std::size_t>;

class Polygon {
public:
    Polygon();
    ~Polygon();

    void addVertex(const vec_t& v);
    void addVertices(const std::vector<vec_t>& v);
    void addEdge(const PolygonEdge& e);

    const std::vector<vec_t>& getVertices() const;
    const std::vector<PolygonEdge>& getEdges() const;

private:
    std::vector<vec_t> vertices;
    std::vector<PolygonEdge> edges;
};

}
