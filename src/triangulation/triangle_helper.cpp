
#include "triangle_helper.h"

#include <Triangle/jrs_triangle.h>
#include <triangle_api.h>

namespace omg {

// TriangleIn implementation
template<typename io_t>
TriangleIn<io_t>::TriangleIn(const Polygon& poly) {

    const std::vector<OpenMesh::Vec2d>& vertices = poly.getVertices();
    const std::vector<PolygonEdge>& edges = poly.getEdges();

    // initialize input triangulateio struct
    io.numberofpoints = vertices.size();
    io.pointlist = new double[vertices.size() * 2];
    // copy vertices
    for (int i = 0; i < vertices.size(); i++) {
        io.pointlist[i * 2 + 0] = vertices[i][0];
        io.pointlist[i * 2 + 1] = vertices[i][1];
    }
    
    io.numberofpointattributes = 0;
    io.pointattributelist = nullptr;
    io.pointmarkerlist = nullptr;

    io.numberofsegments = edges.size();
    io.segmentlist = new int[edges.size() * 2];
    // copy segments
    for (int i = 0; i < edges.size(); i++) {
        io.segmentlist[i * 2 + 0] = edges[i].first;
        io.segmentlist[i * 2 + 1] = edges[i].second;
    }
    io.segmentmarkerlist = nullptr;

    io.numberofholes = 0;
    io.holelist = nullptr;
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
    io.trianglelist = nullptr;
}

template<typename io_t>
TriangleOut<io_t>::~TriangleOut() {
    delete[] io.pointlist;
    delete[] io.trianglelist;
}

template<typename io_t>
void TriangleOut<io_t>::toMesh(Mesh& mesh) const {
    // convert result to OpenMesh
    // add vertices
    std::vector<Mesh::VertexHandle> vertex_handles;
    vertex_handles.reserve(io.numberofpoints);

    Mesh::Point vertex;
    for (int i = 0; i < io.numberofpoints; i++) {
        vertex[0] = io.pointlist[2 * i + 0];
        vertex[1] = io.pointlist[2 * i + 1];
        vertex[2] = 0;  // 3rd component not needed

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

}
