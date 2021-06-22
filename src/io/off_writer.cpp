
#include <OpenMesh/Core/IO/MeshIO.hh>

#include <filesystem>

#include "off_writer.h"

namespace omg {
namespace io {

void writeOff(const std::string& filename, const Mesh& mesh) {
    std::filesystem::path filepath(filename);
    if (filepath.extension() != ".off") {  // TODO: useless restriction, but for now ...
        throw std::runtime_error("Wrong file format: " + filename + " expected: .off");
    }

    if (!OpenMesh::IO::write_mesh(mesh, filename)) {
        throw std::runtime_error("Error writing file: " + filename);
    }
}

}
}
