
#include "polygon.h"

namespace omg
{

Polygon::Polygon()
{
}

Polygon::~Polygon()
{
}

void Polygon::addVertex(const OpenMesh::Vec2d& v)
{
    vertices.push_back(v);
}

void Polygon::addVertices(const std::vector<OpenMesh::Vec2d>& v)
{
    vertices.insert(vertices.end(), v.begin(), v.end());
}

void Polygon::addEdge(const PolygonEdge& e)
{
    edges.push_back(e);
}

const std::vector<OpenMesh::Vec2d>& Polygon::getVertices() const
{
    return vertices;
}

const std::vector<PolygonEdge>& Polygon::getEdges() const
{
    return edges;
}

}
