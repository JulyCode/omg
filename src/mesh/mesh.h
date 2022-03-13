#pragma once

#include <types.h>

#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>

namespace omg {

struct MeshTraits : public OpenMesh::DefaultTraits {
    using Point = vec3_t;
    using Scalar = real_t;

    VertexTraits {
		bool marked;
	};

    VertexAttributes(OpenMesh::Attributes::Status);
	FaceAttributes(OpenMesh::Attributes::Status);
	EdgeAttributes(OpenMesh::Attributes::Status);
};

class Mesh : public OpenMesh::TriMesh_ArrayKernelT<MeshTraits> {
public:
    Mesh() = default;

    inline void mark(const OpenMesh::VertexHandle& v) {
        data(v).marked = true;
    }

    inline bool isMarked(const OpenMesh::VertexHandle& v) {
        return data(v).marked;
    }

    inline void resetMarks() {
	    for (const auto &v : vertices()) {
		    data(v).marked = false;
	    }
    }

    void removeSeparatedSubmeshes();

private:
    real_t faceArea(const OpenMesh::SmartFaceHandle& fh) const;
};

}
