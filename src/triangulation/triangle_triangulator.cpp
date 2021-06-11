
#include "triangle_triangulator.h"

#include <limits>

// defines needed for triangle.h
#ifndef ANSI_DECLARATORS
#define ANSI_DECLARATORS
#endif
#define REAL double
#define VOID void
extern "C" {
    #include <Triangle/triangle.h>
    #include <Triangle/triunsuitable_external.h>
}

namespace omg {

TriangleTriangulator::TriangleTriangulator() {
    // init callback function to interact with Triangle
    set_triunsuitable_callback(triunsuitable);
}

TriangleTriangulator::~TriangleTriangulator() {}

void TriangleTriangulator::generateMesh(const Polygon& outline) {
    const std::vector<OpenMesh::Vec2d>& vertices = outline.getVertices();
    const std::vector<PolygonEdge>& edges = outline.getEdges();

    // Triangle only uses int as size type
    if (vertices.size() == 0 || vertices.size() >= std::numeric_limits<int>::max()) {
        throw std::runtime_error("Invalid number of vertices in outline");
    }
    if (edges.size() == 0 || edges.size() >= std::numeric_limits<int>::max()) {
        throw std::runtime_error("Invalid number of edges in outline");
    }

    triangulateio in, out;

    // initialize input struct
    in.numberofpoints = vertices.size();
    in.pointlist = new double[vertices.size() * 2];
    // copy vertices
    for (int i = 0; i < vertices.size(); i++) {
        in.pointlist[i * 2 + 0] = vertices[i][0];
        in.pointlist[i * 2 + 1] = vertices[i][1];
    }
    
    in.numberofpointattributes = 0;
    in.pointattributelist = nullptr;
    in.pointmarkerlist = nullptr;

    in.numberofsegments = edges.size();
    in.segmentlist = new int[edges.size() * 2];
    // copy segments
    for (int i = 0; i < edges.size(); i++) {
        in.segmentlist[i * 2 + 0] = edges[i].first;
        in.segmentlist[i * 2 + 1] = edges[i].second;
    }
    in.segmentmarkerlist = nullptr;

    in.numberofholes = 0;
    in.holelist = nullptr;
    in.numberofregions = 0;
    in.regionlist = nullptr;

    // initialize output struct
    out.pointlist = nullptr;
    out.trianglelist = nullptr;

    // Q: no console output
    // z: use zero based indexing
    // B: don't output boundary information
    // P: don't output segment information
    // u: use triunsuitable quality check
    // p: triangulate a Planar Straight Line Graph (.poly file)
    char args[] = "zBPup";
    triangulate(args, &in, &out, nullptr);

    // TODO: return triangulation

    // clean up
    delete[] in.pointlist;
    delete[] in.segmentlist;
    trifree(out.pointlist);
    trifree(out.trianglelist);
}

int TriangleTriangulator::triunsuitable(double* v1, double* v2, double* v3, double area) {
    return 0;
}

}
