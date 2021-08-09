#pragma once

#include <optional>
#include <list>
#include <unordered_map>

#include <topology/scalar_field.h>
#include <geometry/he_polygon.h>

namespace omg {

class Boundary {
public:
    Boundary(const BathymetryData& data, const LineGraph& poly, real_t height = 0);

    inline const HEPolygon& getOuter() const { return outer; }

private:
    HEPolygon outer;
    std::vector<HEPolygon> holes;

    const real_t height;
    const BathymetryData& data;
    HEPolygon region;


    using VHandle = LineGraph::VertexHandle;
    using EHandle = LineGraph::EdgeHandle;

    using LineSegment = std::pair<const vec2_t&, const vec2_t&>;  // start and end point of line segment
    using Intersection = std::pair<EHandle, vec2_t>;  // intersected edge and intersection point
    // intersections per region edge
    using IntersectionList = std::unordered_map<HEPolygon::HalfEdgeHandle, std::vector<Intersection>>;

    using AdjacencyList = std::vector<std::list<EHandle>>;  // incident edges per vertex

    void convertToRegion(const LineGraph& poly);

    AdjacencyList getAdjacency(const LineGraph& graph) const;

    void computeIntersections(const LineGraph& coast, IntersectionList& intersections) const;
    std::optional<real_t> lineIntersectFactor(LineSegment l1, LineSegment l2) const;

    bool pointInPolygon(const vec2_t& p, const HEPolygon& poly, const vec2_t& dir = {1, 1}) const;

    void clampToRegion(LineGraph& coast, AdjacencyList& adjacency, const IntersectionList& intersections) const;

    Intersection getNextIntersection(const IntersectionList& intersections, std::size_t edge_idx,
                                 std::size_t intersection_idx, std::vector<vec2_t>& corners) const;

    void cutEdge(LineGraph& coast, AdjacencyList& adjacency, EHandle edge, VHandle cut) const;

    std::vector<HEPolygon> findCycles(const LineGraph& coast, const AdjacencyList& adjacency) const;

    std::size_t findOuterPolygon(const std::vector<HEPolygon>& cycles);

    bool enclosesWater(const HEPolygon& poly) const;
};

}
