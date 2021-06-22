
#include "polygon.h"

namespace omg {

Polygon::Polygon() {}

Polygon::~Polygon() {}

void Polygon::addVertex(const vec_t& v) {
    vertices.push_back(v);
}

void Polygon::addVertices(const std::vector<vec_t>& v) {
    vertices.insert(vertices.end(), v.begin(), v.end());
}

void Polygon::addEdge(const PolygonEdge& e) {
    edges.push_back(e);
}

const std::vector<vec_t>& Polygon::getVertices() const {
    return vertices;
}

const std::vector<PolygonEdge>& Polygon::getEdges() const {
    return edges;
}

}
