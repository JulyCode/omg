#include "poly_reader.h"

#include <fstream>
#include <filesystem>

#include <OpenMesh/Core/Geometry/VectorT.hh>

namespace omg {
namespace io {

struct VertexBuffer {
    std::vector<vec_t> vertices;
    bool zero_based;
};

static void ignoreComments(std::ifstream& file) {
    // read until a line without comment is found
    int c = file.peek();
    while (c != EOF && static_cast<char>(c) == '#') {
        std::string ignore;
        std::getline(file, ignore);
        c = file.peek();
    }
}

static void readVertex(std::ifstream& file, std::size_t& idx_out, vec_t& v_out) {
    ignoreComments(file);
    std::string ignore;
    file >> idx_out >> v_out[0] >> v_out[1];  // read idx, x, y
    std::getline(file, ignore);  // ignore rest of line
}

static void readVertices(std::ifstream& file, VertexBuffer& vb, std::size_t num_vertices) {
    if (num_vertices == 0) {
        return;
    }

    vb.vertices.reserve(num_vertices);
    
    std::size_t idx;
    vec_t vertex(0);

    // read first vertex and check if indices are zero based
    readVertex(file, idx, vertex);
    vb.vertices.push_back(vertex);
    vb.zero_based = (idx == 0);

    // read rest of vertices
    for (std::size_t i = 1; i < num_vertices; i++) {
        readVertex(file, idx, vertex);
        vb.vertices.push_back(vertex);
    }
}

Polygon readPoly(const std::string& filename) {

    const std::filesystem::path file_path(filename);

    if (file_path.extension() != ".poly") {
        throw std::runtime_error("Wrong file format: " + filename + " expected: .poly");
    }

    Polygon poly;

    // open file
    std::ifstream file(filename);
    if (!file.good()) {
        throw std::runtime_error("Error reading file: " + filename);
    }

    ignoreComments(file);
    // read vertices info
    std::size_t num_vertices;
    int dimension, num_attributes, num_boundary_markers;
    file >> num_vertices >> dimension >> num_attributes >> num_boundary_markers;

    // read vertices from this .poly file or a .node file with the same name
    VertexBuffer v_buf;
    if (num_vertices == 0) {

        // create path of .node file
        std::filesystem::path node_path = file_path;
        node_path.replace_extension(".node");

        std::ifstream node_file(node_path);
        if (!node_file.good()) {
            throw std::runtime_error("Error reading file: " + node_path.string());
        }

        ignoreComments(node_file);
        // override vertices info
        node_file >> num_vertices >> dimension >> num_attributes >> num_boundary_markers;

        readVertices(node_file, v_buf, num_vertices);

        node_file.close();
    } else {
        readVertices(file, v_buf, num_vertices);
    }

    poly.addVertices(v_buf.vertices);

    ignoreComments(file);
    // read segments info
    std::size_t num_segments;
    file >> num_segments >> num_boundary_markers;
    
    // read segments
    std::string ignore;
    std::size_t start_vertex, end_vertex;
    PolygonEdge edge;

    for (std::size_t i = 0; i < num_segments; i++) {

        ignoreComments(file);
        file >> ignore >> start_vertex >> end_vertex;
        std::getline(file, ignore);

        // adjust vertex indices to be zero based
        if (!v_buf.zero_based) {
            start_vertex -= 1;
            end_vertex -= 1;
        }

        edge.first = start_vertex;
        edge.second = end_vertex;
        poly.addEdge(edge);
    }

    // TODO: handle holes

    file.close();

    return poly;
}

}
}
