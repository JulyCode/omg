
#include "triangle_triangulator.h"

#include <limits>

#include <Triangle/jrs_triangle.h>

namespace omg {

// save triangulateio wrapper
struct TriangleIn {
    TriangleIn(const Polygon& outline);
    ~TriangleIn();

    const jrs::triangulateio& getData() const;

    jrs::triangulateio io;
};

struct TriangleOut {
    TriangleOut();
    ~TriangleOut();

    jrs::triangulateio& getData() const;

    void toMesh(Mesh& mesh) const;

    jrs::triangulateio io;
};


TriangleTriangulator::TriangleTriangulator() {
    // init callback function to interact with Triangle
    jrs::set_triunsuitable_callback(triunsuitable);
}

TriangleTriangulator::~TriangleTriangulator() {}

void TriangleTriangulator::generateMesh(const Polygon& outline, Mesh& mesh) {
    const std::size_t num_vertices = outline.getVertices().size();
    const std::size_t num_edges = outline.getEdges().size();

    // Triangle only uses int as size type
    if (num_vertices == 0 || num_vertices >= std::numeric_limits<int>::max()) {
        throw std::runtime_error("Invalid number of vertices in outline");
    }
    if (num_edges == 0 || num_edges >= std::numeric_limits<int>::max()) {
        throw std::runtime_error("Invalid number of edges in outline");
    }

    TriangleIn in(outline);
    TriangleOut out;

    // Q: no console output
    // z: use zero based indexing
    // B: don't output boundary information
    // P: don't output segment information
    // u: use triunsuitable quality check
    // p: triangulate a Planar Straight Line Graph (.poly file)
    // D: conforming Delaunay
    // a0.5: area constraint of 0.5
    // q25: minimum angle of 25 degrees, maximum 180 - 2 * 25
    char args[] = "zBPupDa0.5q25";
    jrs::triangulate(args, &in.io, &out.io, nullptr);

    out.toMesh(mesh);
}

int TriangleTriangulator::triunsuitable(double* v1, double* v2, double* v3, double area) {
    return 0;
}


TriangleIn::TriangleIn(const Polygon& poly) {

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

TriangleIn::~TriangleIn() {
    delete[] io.pointlist;
    delete[] io.segmentlist;
}

TriangleOut::TriangleOut() {
    io.pointlist = nullptr;
    io.trianglelist = nullptr;
}

TriangleOut::~TriangleOut() {
    jrs::trifree(io.pointlist);
    jrs::trifree(io.trianglelist);
}

void TriangleOut::toMesh(Mesh& mesh) const {
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

}
