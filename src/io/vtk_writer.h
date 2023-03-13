#pragma once

#include <topology/scalar_field.h>
#include <geometry/line_graph.h>
#include <mesh/mesh.h>

namespace omg {
namespace io {

template<typename T>
void writeLegacyVTK(const std::string& filename, const ScalarField<T>& data, bool binary = false);

void writeLegacyVTK(const std::string& filename, const LineGraph& poly);

namespace internal {

void writeLegacyVTK(const std::string& filename, const OpenMesh::PolyConnectivity& conn,
					const std::function<std::string(OpenMesh::VertexHandle)>& point);
}

template<typename Traits>
void writeLegacyVTK(const std::string& filename, const OpenMesh::TriMesh_ArrayKernelT<Traits>& mesh) {
    auto point = [&mesh](OpenMesh::VertexHandle vh) {
        auto p = mesh.point(vh);
        std::stringstream ss;
        ss << p[0] << " " << p[1] << " " << p[2];
        return ss.str();
    };
    internal::writeLegacyVTK(filename, mesh, point);
}

}
}
