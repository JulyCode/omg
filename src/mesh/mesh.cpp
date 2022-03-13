
#include "mesh.h"

#include <vector>
#include <unordered_set>

namespace omg {


real_t Mesh::faceArea(const OpenMesh::SmartFaceHandle& fh) const {
    const auto vertices = fh.vertices().to_vector();
    const vec3_t a = point(vertices[0]) - point(vertices[1]);
    const vec3_t b = point(vertices[2]) - point(vertices[1]);

    return 0.5 * std::abs(a[0] * b[1] - a[1] * b[0]);
}

void Mesh::removeSeparatedSubmeshes() {
    struct Submesh {
        real_t area;
        std::vector<Mesh::FaceHandle> faces;
    };

    // find submeshes that are not connected
    std::vector<Submesh> meshes;
    std::unordered_set<int> visited;

    for (const auto& fh : faces()) {
        if (visited.find(fh.idx()) == visited.end()) {

            // unseen face -> create new submesh
            meshes.push_back(Submesh());
            Submesh& sm = meshes.back();
            sm.area = 0;

            // floodfill
            std::vector<OpenMesh::SmartFaceHandle> stack;
            stack.push_back(fh);

            while (!stack.empty()) {
                const auto& face = stack.back();
                stack.pop_back();

                if (visited.find(face.idx()) != visited.end()) {
                    continue;
                }

                // add face to submesh
                sm.faces.push_back(face);
                sm.area += faceArea(face);

                visited.insert(face.idx());

                // push neighbors to stack
                for (const auto& neighbor : face.faces()) {
                    stack.push_back(neighbor);
                }
            }
        }
    }

    assert(meshes.size() > 0);

    // find largest submesh
    std::size_t idx = 0;
    real_t largest = meshes[0].area;

    for (std::size_t i = 1; i < meshes.size(); i++) {
        if (meshes[i].area > largest) {
            largest = meshes[i].area;
            idx = i;
        }
    }

    // remove all but the largest
    for (std::size_t i = 0; i < meshes.size(); i++) {
        if (i == idx) {
            continue;
        }

        for (const auto& fh : meshes[i].faces) {
            delete_face(fh);
        }
    }

    garbage_collection();
}

}
