#pragma once

#include <vector>

#include <OpenMesh/Core/Geometry/VectorT.hh>

namespace omg
{

using PolygonEdge = std::pair<std::size_t, std::size_t>;

class Polygon
{
public:
    Polygon();
    ~Polygon();

    void addVertex(const OpenMesh::Vec2d& v);
    void addVertices(const std::vector<OpenMesh::Vec2d>& v);
    void addEdge(const PolygonEdge& e);

    const std::vector<OpenMesh::Vec2d>& getVertices() const;
    const std::vector<PolygonEdge>& getEdges() const;

private:
    std::vector<OpenMesh::Vec2d> vertices;
    std::vector<PolygonEdge> edges;
};

}
