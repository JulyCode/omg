#pragma once

#include <util.h>
#include <geometry/he_polygon.h>
#include <geometry/line_graph.h>

namespace omg {

class Boundary {
public:
    Boundary() = default;

    inline void setOuter(const HEPolygon& poly) { outer = poly; }
    inline void addIsland(const HEPolygon& poly) { islands.push_back(poly); }

    inline const HEPolygon& getOuter() const { return outer; }
    inline const std::vector<HEPolygon>& getIslands() const { return islands; }

    inline bool hasIntersections() const {
        ScopeTimer timer("Has intersections");

        std::vector<HEPolygon> polys = islands;
        polys.push_back(outer);

        omg::LineGraph complete = LineGraph::combinePolygons(polys);

        return complete.hasSelfIntersection();
    }

private:
    HEPolygon outer;
    std::vector<HEPolygon> islands;
};

}
