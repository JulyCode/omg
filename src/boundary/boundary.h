#pragma once

#include <list>
#include <unordered_map>

#include <topology/scalar_field.h>
#include <geometry/he_polygon.h>
#include <size_function/size_function.h>

namespace omg {

class Boundary {
public:
    Boundary(const BathymetryData& data, const LineGraph& poly, const SizeFunction& size, real_t height = 0);

    inline const HEPolygon& getOuter() const { return outer; }
    inline const std::vector<HEPolygon>& getHoles() const { return holes; }

private:
    HEPolygon outer;
    std::vector<HEPolygon> holes;

    const real_t height;
    const BathymetryData& data;
    HEPolygon region;
    const SizeFunction& size;


    using VHandle = LineGraph::VertexHandle;
    using EHandle = LineGraph::EdgeHandle;

    using Intersection = std::pair<EHandle, vec2_t>;  // intersected edge and intersection point

    // intersections per region edge
    using IntersectionList = std::unordered_map<HEPolygon::HalfEdgeHandle, std::vector<Intersection>>;

    using AdjacencyList = std::vector<std::list<EHandle>>;  // incident edges per vertex


    void convertToRegion(const LineGraph& poly);

    AdjacencyList getAdjacency(const LineGraph& graph) const;

    void computeIntersections(const LineGraph& coast, IntersectionList& intersections) const;

    void clampToRegion(LineGraph& coast, AdjacencyList& adjacency, const IntersectionList& intersections) const;

    Intersection getNextIntersection(const IntersectionList& intersections, std::size_t edge_idx,
                                 std::size_t intersection_idx, std::vector<vec2_t>& corners) const;

    void cutEdge(LineGraph& coast, AdjacencyList& adjacency, EHandle edge, VHandle cut) const;

    std::vector<HEPolygon> findCycles(const LineGraph& coast, const AdjacencyList& adjacency) const;

    bool enclosesWater(const HEPolygon& poly) const;

    std::size_t findOuterPolygon(const std::vector<HEPolygon>& cycles);
};

}
