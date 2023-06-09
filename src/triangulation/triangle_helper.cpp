
#include "triangle_helper.h"

#include <iostream>

#include <Triangle/jrs_triangle.h>
#include <triangle_api.h>

#include <io/vtk_writer.h>

namespace omg {

static void restrictToInt(const LineGraph& outline) {
    const std::size_t num_vertices = outline.getPoints().size();
    const std::size_t num_edges = outline.getEdges().size();

    // throw error if sizes exceed ints
    if (!fitsInt(num_vertices)) {
        throw std::runtime_error("Too many vertices in outline");
    }
    if (!fitsInt(num_edges)) {
        throw std::runtime_error("Too many edges in outline");
    }
}

// TriangleIn implementation
template<typename io_t>
TriangleIn<io_t>::TriangleIn(const Boundary& boundary) {

    // combine outer and holes
    std::vector<HEPolygon> polys = boundary.getIslands();
    polys.push_back(boundary.getOuter());
    omg::LineGraph outline = LineGraph::combinePolygons(polys);

    restrictToInt(outline);

    const std::vector<vec2_t>& points = outline.getPoints();
    const std::vector<LineGraph::Edge>& edges = outline.getEdges();

    // initialize input triangulateio struct
    io.numberofpoints = points.size();
    io.pointlist = new double[points.size() * 2];
    // copy points
    for (std::size_t i = 0; i < points.size(); i++) {
        io.pointlist[i * 2 + 0] = points[i][0];
        io.pointlist[i * 2 + 1] = points[i][1];
    }

    io.numberofpointattributes = 0;
    io.pointattributelist = nullptr;
    io.pointmarkerlist = nullptr;

    io.numberofsegments = edges.size();
    io.segmentlist = new int[edges.size() * 2];
    // copy segments
    for (std::size_t i = 0; i < edges.size(); i++) {
        io.segmentlist[i * 2 + 0] = edges[i].first;
        io.segmentlist[i * 2 + 1] = edges[i].second;
    }
    io.segmentmarkerlist = nullptr;

    // create holes
    const std::vector<HEPolygon>& holes = boundary.getIslands();
    io.numberofholes = holes.size();
    io.holelist = new double[io.numberofholes * 2];
    for (int i = 0; i < io.numberofholes; i++) {

        // get a point inside the polygon
        const vec2_t center = holes[i].getPointInPolygon();

        io.holelist[i * 2 + 0] = center[0];
        io.holelist[i * 2 + 1] = center[1];
    }

    io.numberofregions = 0;
    io.regionlist = nullptr;
}

template<typename io_t>
TriangleIn<io_t>::~TriangleIn() {
    delete[] io.pointlist;
    delete[] io.segmentlist;
}


// TriangleOut implementation
template<typename io_t>
TriangleOut<io_t>::TriangleOut() {
    io.pointlist = nullptr;
    io.pointattributelist = nullptr;
    io.pointmarkerlist = nullptr;
    io.trianglelist = nullptr;
    io.triangleattributelist = nullptr;
    io.trianglearealist = nullptr;
    io.neighborlist = nullptr;
    io.segmentlist = nullptr;
    io.segmentmarkerlist = nullptr;
    io.holelist = nullptr;
    io.regionlist = nullptr;
    io.edgelist = nullptr;
    io.edgemarkerlist = nullptr;
}

template<typename io_t>
TriangleOut<io_t>::~TriangleOut() {
    delete[] io.pointlist;
    delete[] io.trianglelist;
}

template<typename io_t>
void TriangleOut<io_t>::toMesh(Mesh& mesh) const {  // TODO: error checking

    if (io.numberofcorners != 3) {
        throw std::runtime_error("Invalid number of corners");
    }

    // convert result to OpenMesh
    // add points
    std::vector<Mesh::VertexHandle> vertex_handles;
    vertex_handles.reserve(io.numberofpoints);

    Mesh::Point vertex(0);
    for (int i = 0; i < io.numberofpoints; i++) {
        vertex[0] = io.pointlist[2 * i + 0];
        vertex[1] = io.pointlist[2 * i + 1];

        vertex_handles.push_back(mesh.add_vertex(vertex));
    }

    // add triangles
    Mesh::VertexHandle v0, v1, v2;
    for (int i = 0; i < io.numberoftriangles; i++) {
        v0 = vertex_handles[io.trianglelist[3 * i + 0]];
        v1 = vertex_handles[io.trianglelist[3 * i + 1]];
        v2 = vertex_handles[io.trianglelist[3 * i + 2]];

        mesh.add_face(v0, v1, v2);
    }
}

template class TriangleIn<jrs::triangulateio>;
template class TriangleOut<jrs::triangulateio>;

template class TriangleIn<triangleio>;
template class TriangleOut<triangleio>;

}
