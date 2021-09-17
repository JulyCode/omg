#pragma once

#include <unordered_map>

#include <topology/scalar_field.h>
#include <geometry/line_graph.h>
#include <size_function/size_function.h>

namespace omg {

class Boundary {
public:
    Boundary(const BathymetryData& data, const LineGraph& poly, const SizeFunction& size);

    void generate(real_t height = 0, bool simplify = true);

    inline const HEPolygon& getOuter() const { return outer; }
    inline const std::vector<HEPolygon>& getIslands() const { return islands; }

    bool hasIntersections() const;

private:
    HEPolygon outer;
    std::vector<HEPolygon> islands;

    real_t height;
    const BathymetryData& data;
    HEPolygon region;
    const SizeFunction& size;


    using VHandle = LineGraph::VertexHandle;
    using EHandle = LineGraph::EdgeHandle;

    using Intersection = std::pair<EHandle, vec2_t>;  // intersected edge and intersection point

    // intersections per region edge
    using IntersectionList = std::unordered_map<HEPolygon::HalfEdgeHandle, std::vector<Intersection>>;

    using AdjacencyList = LineGraph::AdjacencyList;

    void convertToRegion(const LineGraph& poly);

    void computeIntersections(const LineGraph& coast, IntersectionList& intersections) const;

    void clampToRegion(LineGraph& coast, AdjacencyList& adjacency, const IntersectionList& intersections) const;

    Intersection getNextIntersection(const IntersectionList& intersections, std::size_t edge_idx,
                                 std::size_t intersection_idx, std::vector<vec2_t>& corners) const;

    void cutEdge(LineGraph& coast, AdjacencyList& adjacency, EHandle edge, VHandle cut) const;

    std::vector<HEPolygon> findCycles(const LineGraph& coast, const AdjacencyList& adjacency) const;

    bool enclosesWater(const HEPolygon& poly) const;

    std::size_t findOuterPolygon(const std::vector<HEPolygon>& cycles);

    void findIslands(std::vector<HEPolygon>& cycles, bool simplify);
};

}
