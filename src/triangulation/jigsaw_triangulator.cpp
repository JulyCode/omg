
#include "jigsaw_triangulator.h"

#include <iostream>

#include <util.h>

#include <jigsaw/inc/lib_jigsaw.h>

namespace omg {

JigsawTriangulator::JigsawTriangulator() {}

using VertexBuffer = std::vector<jigsaw_VERT2_t>;
using EdgeBuffer = std::vector<jigsaw_EDGE2_t>;
using BoundBuffer = std::vector<jigsaw_BOUND_t>;
using CoordBuffer = std::vector<::real_t>;
using ValueBuffer = std::vector<::fp32_t>;

static void convertBoundary(const Boundary& boundary, jigsaw_msh_t& coast,
                            VertexBuffer& vertices, EdgeBuffer& edges, BoundBuffer& bounds) {

    const HEPolygon& outer = boundary.getOuter();

    std::vector<HEPolygon> polys = boundary.getHoles();
    polys.push_back(outer);
    omg::LineGraph outline = LineGraph::combinePolygons(polys);

    if (outline.numVertices() > static_cast<std::size_t>(std::numeric_limits<indx_t>::max())) {
        throw std::runtime_error("Too many vertices in outline");
    }
    if (outline.numEdges() > static_cast<std::size_t>(std::numeric_limits<indx_t>::max())) {
        throw std::runtime_error("Too many edges in outline");
    }

    // copy vertices
    vertices.reserve(outline.numVertices());

    for (const vec2_t& p : outline.getPoints()) {
        vertices.push_back({{p[0], p[1]}, 0});
    }

    // copy edges
    edges.reserve(outline.numEdges());

    for (const LineGraph::Edge& e : outline.getEdges()) {

        const indx_t v0 = e.first;
        const indx_t v1 = e.second;
        edges.push_back({{v0, v1}, 0});
    }

    // create bounds
    bounds.reserve(outer.numHalfEdges());

    for (std::size_t i = 0; i < outer.numHalfEdges(); i++) {

        indx_t idx = outline.numEdges() - i - 1;
        bounds.push_back({0, idx, JIGSAW_EDGE2_TAG});
    }

    coast._flags = JIGSAW_EUCLIDEAN_MESH;

    coast._vert2._data = vertices.data();
    coast._vert2._size = vertices.size();

    coast._edge2._data = edges.data();
    coast._edge2._size = edges.size();

    // TODO: is this needed?
    // coast._bound._data = bounds.data();
    // coast._bound._size = bounds.size();
}

static void convertSize(const SizeFunction& size, jigsaw_msh_t& h_fun,
                        CoordBuffer& x_buf, CoordBuffer& y_buf, ValueBuffer& value) {

    const size2_t& grid_size = size.getGridSize();

    x_buf.reserve(grid_size[0]);
    y_buf.reserve(grid_size[1]);
    value.reserve(grid_size[0] * grid_size[1]);

    for (std::size_t x = 0; x < grid_size[0]; x++) {

        const vec2_t p = size.getPoint({x, 0});
        x_buf.push_back(p[0]);
    }
    for (std::size_t y = 0; y < grid_size[1]; y++) {

        const vec2_t p = size.getPoint({0, y});
        y_buf.push_back(p[1]);
    }

    for (std::size_t x = 0; x < grid_size[0]; x++) {
        for (std::size_t y = 0; y < grid_size[1]; y++) {

            value.push_back(metersToDegrees(size.grid(x, y)));  // TODO: save size in degrees?
        }
    }

    h_fun._flags = JIGSAW_EUCLIDEAN_GRID;

    h_fun._xgrid._data = x_buf.data();
    h_fun._xgrid._size = x_buf.size();

    h_fun._ygrid._data = y_buf.data();
    h_fun._ygrid._size = y_buf.size();

    h_fun._value._data = value.data();
    h_fun._value._size = value.size();
}

static void convertToMesh(const jigsaw_msh_t& jig_mesh, Mesh& out_mesh) {
    // add points
    std::vector<Mesh::VertexHandle> vertex_handles;
    vertex_handles.reserve(jig_mesh._vert2._size);

    for (std::size_t i = 0; i < jig_mesh._vert2._size; i++) {

        const jigsaw_VERT2_t& vert = jig_mesh._vert2._data[i];

        vertex_handles.push_back(out_mesh.add_vertex({vert._ppos[0], vert._ppos[1], 0}));
    }

    // add triangles
    Mesh::VertexHandle v0, v1, v2;
    for (std::size_t i = 0; i < jig_mesh._tria3._size; i++) {

        const auto& tri = jig_mesh._tria3._data[i];

        v0 = vertex_handles[tri._node[0]];
        v1 = vertex_handles[tri._node[1]];
        v2 = vertex_handles[tri._node[2]];

        out_mesh.add_face(v0, v1, v2);
    }
}

void JigsawTriangulator::generateMesh(const Boundary& boundary, const SizeFunction& size, Mesh& out_mesh) {
    ScopeTimer timer("Jigsaw generate mesh");

    jigsaw_jig_t jig;
    jigsaw_init_jig_t(&jig);

    jig._verbosity = 1;
    jig._mesh_dims = 2;

    jigsaw_msh_t coast, mesh, h_fun;
    jigsaw_init_msh_t(&coast);
    jigsaw_init_msh_t(&mesh);
    jigsaw_init_msh_t(&h_fun);

    // buffers need same lifetime as mesh
    VertexBuffer vertices;
    EdgeBuffer edges;
    BoundBuffer bounds;
    CoordBuffer x_buf, y_buf;
    ValueBuffer value;

    convertBoundary(boundary, coast, vertices, edges, bounds);

    // size function
    convertSize(size, h_fun, x_buf, y_buf, value);
    jig._hfun_hmax = metersToDegrees(size.getMax());
    jig._hfun_hmin = 0;
    jig._hfun_scal = JIGSAW_HFUN_ABSOLUTE;

    int retv = jigsaw(&jig, &coast, NULL, &h_fun, &mesh);

    if (retv != 0) {
        throw std::runtime_error("jigsaw error " + std::to_string(retv));
    }

    convertToMesh(mesh, out_mesh);

    jigsaw_free_msh_t(&mesh);
}

}
