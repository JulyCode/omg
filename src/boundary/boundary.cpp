
#include "boundary.h"

#include <iostream>
#include <mutex>

#include <boundary/marching_quads.h>
#include <boundary/simplification.h>
#include <geometry/line_intersection.h>

#include <io/vtk_writer.h>

namespace omg {

Boundary::Boundary(const BathymetryData& data, const LineGraph& poly, const SizeFunction& size, real_t height)
    : height(height), data(data), size(size) {

    // convert LineGraph to region HEPolygon
    convertToRegion(poly);

    LineGraph coast = marchingQuads(data, height);
    // io::writeLegacyVTK("../../apps/coast.vtk", coast);

    // search adjacent edges per vertex
    AdjacencyList adjacency = getAdjacency(coast);

    // insert intersections of region with coast
    IntersectionList intersections;
    computeIntersections(coast, intersections);

    clampToRegion(coast, adjacency, intersections);

    std::vector<HEPolygon> cycles = findCycles(coast, adjacency);

    // remove polygons that are outside the region
    // TODO: doesn't work if a cut point is selected
    for (auto it = cycles.begin(); it != cycles.end();) {
        const vec2_t& start_point = it->point(*it->vertices().begin());

        if (!region.pointInPolygon(start_point)) {
            it = cycles.erase(it);
        } else {
            ++it;
        }
    }

    // find and remove the outer polygon
    // special case: region polygon is completely on water
    std::size_t num_intersections = 0;
    for (const auto& e : intersections) {
        num_intersections += e.second.size();
    }

    const vec2_t& first_corner = region.startPoint(*region.halfEdges().begin());
    const bool is_water = data.getValue<real_t>(first_corner) < height;

    // no intersections and one corner is below coast height
    if (num_intersections == 0 && is_water) {
        // region is outer boundary
        outer = region;
    } else {
        const std::size_t outer_idx = findOuterPolygon(cycles);
        outer = std::move(cycles[outer_idx]);
        cycles.erase(cycles.begin() + outer_idx);
    }

    std::cout << outer.numVertices() << " vertices" << std::endl;
    std::cout << cycles.size() << " cycles" << std::endl;

    // move the islands to holes
    std::mutex hole_mutex;

    #pragma omp parallel for
    for (std::size_t i = 0; i < cycles.size(); i++) {
        HEPolygon& c = cycles[i];

        const vec2_t& start_point = c.point(*c.vertices().begin());

        if (!enclosesWater(c) && outer.pointInPolygon(start_point)) {

            const std::lock_guard lock(hole_mutex);
            holes.push_back(std::move(c));
        }
    }
    std::cout << holes.size() << " holes" << std::endl;

    // io::writeLegacyVTK("../../apps/complete.vtk", complete);
}

bool Boundary::hasIntersections() const {
    std::vector<HEPolygon> polys = holes;
    polys.push_back(outer);

    omg::LineGraph complete = LineGraph::combinePolygons(polys);

    return complete.hasSelfIntersection();
}

void Boundary::simplify() {
    // simplify outer
    omg::simplifyPolygon(outer, size);
    if (outer.isDegenerated()) {
        throw std::runtime_error("outer boundary is degenerated");
    }
    outer.garbageCollect();

    std::cout << outer.numVertices() << " vertices" << std::endl;

    // simplify holes
    for (auto it = holes.begin(); it != holes.end();) {

        HEPolygon& c = *it;
        omg::simplifyPolygon(c, size);

        if (!c.isDegenerated()) {
            c.garbageCollect();

            ++it;
        } else {
            it = holes.erase(it);  // remove if degenerated
        }
    }

    std::cout << holes.size() << " holes" << std::endl;
}

void Boundary::convertToRegion(const LineGraph& poly) {
    // check for self-intersections
    if (poly.hasSelfIntersection()) {
        throw std::runtime_error("region polygon must not intersect itself");
    }

    // check if non-manifold
    AdjacencyList adjacency = getAdjacency(poly);
    for (const auto& l : adjacency) {
        if (l.size() != 2) {
            throw std::runtime_error("region polygon is not manifold");
        }
    }

    // check for multiple cycles
    std::vector<HEPolygon> cycles = findCycles(poly, adjacency);
    if (cycles.size() != 1) {
        throw std::runtime_error("region polygon must have only one cycle");
    }
    region = std::move(cycles[0]);
}

Boundary::AdjacencyList Boundary::getAdjacency(const LineGraph& graph) const {
    AdjacencyList adjacency(graph.numVertices());

    for (EHandle eh = 0; eh < graph.numEdges(); eh++) {

        adjacency[graph.getEdge(eh).first].push_back(eh);
        adjacency[graph.getEdge(eh).second].push_back(eh);
    }

    return adjacency;
}

void Boundary::computeIntersections(const LineGraph& coast, IntersectionList& intersections) const {
    // TODO: special cases

    std::vector<real_t> t_list;

    // compute intersections ordered along region lines
    for (HEPolygon::HalfEdgeHandle r_eh : region.halfEdgesOrdered()) {

        // region line
        const LineSegment l1 = {region.startPoint(r_eh), region.endPoint(r_eh)};

        // intersections on this edge
        std::vector<Intersection> edge_ints;

        for (EHandle c_eh = 0; c_eh < coast.numEdges(); c_eh++) {

            // coast line
            const LineGraph::Edge& c_edge = coast.getEdge(c_eh);
            const LineSegment l2 = {coast.getPoint(c_edge.first), coast.getPoint(c_edge.second)};

            std::optional<real_t> t = lineIntersectionFactor(l1, l2);
            // intersection found
            if (t) {
                const vec2_t point = l1.first + (*t) * (l1.second - l1.first);

                // insertion sort
                bool found = false;
                for (std::size_t i = 0; i < t_list.size(); i++) {
                    if (*t < t_list[i]) {
                        // insert at position i
                        t_list.insert(t_list.begin() + i, *t);

                        edge_ints.insert(edge_ints.begin() + i, {c_eh, point});
                        found = true;
                        break;
                    }
                }

                // insert at end
                if (!found) {
                    t_list.push_back(*t);
                    edge_ints.push_back({c_eh, point});
                }
            }
        }
        intersections[r_eh] = edge_ints;
        t_list.clear();
    }
}

void Boundary::clampToRegion(LineGraph& coast, AdjacencyList& adjacency, const IntersectionList& intersections) const {

    if (intersections.size() % 2 != 0) {
        throw std::runtime_error("Odd number of intersections");
    }

    // get first region corner
    const vec2_t& first_corner = region.startPoint(*region.halfEdges().begin());
    // if the corner is below iso height, cut the coast off
    // and use the region polygon from the last to the first intersection
    const bool use_region_boundary = data.getValue<real_t>(first_corner) < height;

    // skip every second intersection, because it was already used
    // use_region_boundary determines if the even or odd ones are skipped
    bool skip_intersection = use_region_boundary;

    // iterate over intersections
    for (HEPolygon::HalfEdgeHandle heh : region.halfEdgesOrdered()) {

        // intersections on this edge
        const std::vector<Intersection>& edge_ints = intersections.at(heh);

        for (std::size_t intersection_idx = 0; intersection_idx < edge_ints.size(); intersection_idx++) {

            if (!skip_intersection) {
                const Intersection& intersection = edge_ints[intersection_idx];

                // get next intersection and corners on the way
                std::vector<vec2_t> corners;
                Intersection next = getNextIntersection(intersections, heh, intersection_idx, corners);

                // add new points to coast
                std::vector<VHandle> new_vertices;

                const VHandle start_cut = coast.addVertex(intersection.second);
                const VHandle end_cut = coast.addVertex(next.second);

                new_vertices.push_back(start_cut);
                for (const vec2_t& c : corners) {
                    new_vertices.push_back(coast.addVertex(c));
                }
                new_vertices.push_back(end_cut);

                // resize adjacency
                adjacency.resize(adjacency.size() + new_vertices.size());

                // add new edges
                for (std::size_t i = 0; i < new_vertices.size() - 1; i++) {

                    const VHandle v_i = new_vertices[i];
                    const VHandle v_j = new_vertices[i + 1];

                    const EHandle new_edge = coast.addEdge(v_i, v_j);
                    adjacency[v_i].push_back(new_edge);
                    adjacency[v_j].push_back(new_edge);
                }

                // remove the intersected edges and connect the new points to the coast
                cutEdge(coast, adjacency, intersection.first, start_cut);
                cutEdge(coast, adjacency, next.first, end_cut);
            }

            // flip
            skip_intersection = !skip_intersection;
        }
    }
}

Boundary::Intersection Boundary::getNextIntersection(const IntersectionList& intersections,
                                                     HEPolygon::HalfEdgeHandle heh, std::size_t intersection_idx,
                                                     std::vector<vec2_t>& corners) const {

    // next intersection is on this edge
    if (intersection_idx + 1 < intersections.at(heh).size()) {
        return intersections.at(heh)[intersection_idx + 1];
    } else {
        // search next edges
        // loop around polygon starting from heh
        for (HEPolygon::HalfEdgeHandle h : region.halfEdgesOrdered(region.nextHalfEdge(heh))) {

            // add corner to the list
            corners.push_back(region.startPoint(h));

            // check for intersection
            if (!intersections.at(h).empty()) {
                return intersections.at(h).front();
            }
        }
    }
    throw std::runtime_error("no next intersection found");
}

void Boundary::cutEdge(LineGraph& coast, AdjacencyList& adjacency, EHandle edge, VHandle cut) const {
    // adjust adjacency list
    // throw vertex outside away and update the vertex inside

    // get vertices of cut edge
    const VHandle v1 = coast.getEdge(edge).first;
    const VHandle v2 = coast.getEdge(edge).second;

    // remove the cut edge
    adjacency[v1].remove(edge);
    adjacency[v2].remove(edge);

    // connect the vertex inside to the cut
    VHandle inside;
    if (region.pointInPolygon(coast.getPoint(v1))) {
        inside = v1;
    } else {
        inside = v2;
    }

    const EHandle e = coast.addEdge(inside, cut);
    adjacency[inside].push_back(e);
    adjacency[cut].push_back(e);
}

std::vector<HEPolygon> Boundary::findCycles(const LineGraph& coast, const AdjacencyList& adjacency) const {
    std::vector<HEPolygon> cycles;

    std::vector<bool> done(coast.numVertices());  // finished vertices
    std::unordered_set<VHandle> visited;  // for fast lookup
    std::vector<vec2_t> path;

    for (VHandle start = 0; start < coast.numVertices(); start++) {
        if (done[start]) {  // if this vertex is already done, skip it
            continue;
        }

        // search for cycle
        VHandle vertex = start;
        EHandle last_edge = coast.numEdges();

        while (visited.find(vertex) == visited.end()) {
            visited.insert(vertex);
            path.push_back(coast.getPoint(vertex));

            const std::list<EHandle>& edges = adjacency[vertex];

            if (edges.size() == 1 || done[vertex]) {
                // no cycle found, ignore all visited vertices
                visited.clear();
                break;
            }

            done[vertex] = true;

            if (edges.size() > 2) {
                // non manifold, should not happen
                std::cout << "non manifold" << std::endl;
            }

            // get next edge
            EHandle next_edge = edges.front();
            if (next_edge == last_edge) {
                next_edge = edges.back();
            }
            last_edge = next_edge;

            // get next vertex
            VHandle next_vertex = coast.getEdge(next_edge).first;
            if (next_vertex == vertex) {
                next_vertex = coast.getEdge(next_edge).second;
            }
            vertex = next_vertex;
        }

        if (visited.size() > 2) {
            HEPolygon poly(path);
            cycles.push_back(poly);
        }

        visited.clear();
        path.clear();
    }

    return cycles;
}

std::size_t Boundary::findOuterPolygon(const std::vector<HEPolygon>& cycles) {
    // TODO: fix error, when smaller lake is selected
    if (cycles.empty()) {
        throw std::runtime_error("cycles is empty");
    }

    bool init = true;

    // find the polygon with the largest area
    std::size_t largest = 0;
    real_t largest_area = cycles[0].computeArea();

    for (std::size_t i = 0; i < cycles.size(); i++) {

        // skip polygons enclosing land
        if (!enclosesWater(cycles[i])) {
            continue;
        }

        const real_t area = cycles[i].computeArea();
        if (area > largest_area) {
            largest = i;
            largest_area = area;

            init = false;
        }
    }
    std::cout << "area " << largest_area << std::endl;

    if (init) {
        throw std::runtime_error("no cycle in the region encloses water");
    }

    return largest;
}

bool Boundary::enclosesWater(const HEPolygon& poly) const {
    // TODO: still not completely working, tiny lakes are not correctly identified
    // test if the polygon surrounds water or land

    for (HEPolygon::HalfEdgeHandle heh : poly.halfEdges()) {

        // get two consecutive points
        const vec2_t& p1 = poly.startPoint(heh);
        const vec2_t& p2 = poly.endPoint(heh);

        // ignore points that are cut corners of the region polygon
        if (data.getValue<real_t>(p1) < height - 0.1 || data.getValue<real_t>(p2) < height - 0.1) {
            continue;
        }

        // get gradient at p1
        const vec2_t grad = data.getGradient(p1);

        // is gradient pointing outwards
        return toVec3(p2 - p1).cross(toVec3(grad))[2] < 0;
    }
    throw std::runtime_error("cannot determine if polygon encloses water");
}

}
