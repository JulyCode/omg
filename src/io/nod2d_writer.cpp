
#include <filesystem>
#include <fstream>
#include <unordered_map>

#include "nod2d_writer.h"

namespace omg {
namespace io {

void writeNod2D(const Mesh& mesh, const BathymetryData& topo, const std::string& name, bool zero_based) {

    std::string elem2d_filename, nod2d_filename, nodhn_filename;
    if (!name.empty()) {
        elem2d_filename = name + "_";
        nod2d_filename = name + "_";
        nodhn_filename = name + "_";
    }
    elem2d_filename += "elem2d.out";
    nod2d_filename += "nod2d.out";
    nodhn_filename += "nodhn.out";

    std::size_t counter = zero_based ? 0 : 1;
    std::unordered_map<std::size_t, std::size_t> vertex_table;  // index translation table

    // write vertices and heights
    std::ofstream nod2d_file(nod2d_filename);
    std::ofstream nodhn_file(nodhn_filename);

    nod2d_file << std::setprecision(15);
    nodhn_file << std::setprecision(15);

    nod2d_file << mesh.n_vertices() << "\n";

    for (auto vh : mesh.vertices()) {

        vertex_table[vh.idx()] = counter;

        const vec3_t& point = mesh.point(vh);
        nod2d_file << counter << " " << point[0] << " " << point[1] << " 0\n";  // TODO: fix boundary marker

        nodhn_file << topo.getValue<real_t>(toVec2(point)) << "\n";

        counter++;
    }
    nod2d_file.close();
    nodhn_file.close();

    // write triangles
    std::ofstream elem2d_file(elem2d_filename);

    elem2d_file << mesh.n_faces() << "\n";

    for (auto fh : mesh.faces()) {

        bool first = true;
        for (auto vh : fh.vertices()) {
            if (!first) {
                elem2d_file << " ";
            }
            elem2d_file << vertex_table[vh.idx()];
            first = false;
        }

        elem2d_file << "\n";
    }
    elem2d_file.close();
}

}
}
