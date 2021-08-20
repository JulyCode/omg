
#include "remeshing.h"

#include <iostream>

#include <util.h>

namespace omg {

IsotropicRemeshing::IsotropicRemeshing(const SizeFunction& size, real_t min_size_factor, real_t max_size_factor)
    : size(size), min_size_factor(min_size_factor), max_size_factor(max_size_factor) {}

void IsotropicRemeshing::remesh(Mesh& mesh, unsigned int iterations) const {

    mesh.request_vertex_status();
	mesh.request_edge_status();
	mesh.request_face_status();

    for (unsigned int i = 0; i < iterations; i++) {

        mesh.resetMarks();

        splitEdges(mesh);
		collapseEdges(mesh);

        equalizeValences(mesh);

        mesh.garbage_collection();

        smoothVertices(mesh);
    }

    mesh.release_vertex_status();
	mesh.release_edge_status();
	mesh.release_face_status();
}

void IsotropicRemeshing::splitEdges(Mesh& mesh) const {
    int cnt = 0;
    for (const auto& eh : mesh.edges()) {

        if (eh.is_boundary()) {
            continue;
        }

        // incident points
		const vec2_t& p0 = toVec2(mesh.point(eh.v0()));
		const vec2_t& p1 = toVec2(mesh.point(eh.v1()));

		const vec2_t diff = p1 - p0;

        const vec2_t center = p0 + diff / 2;
        const real_t max_length = max_size_factor * size.getValue(center);  // TODO: use minimum of p0 and p1?

        // check edge length
		if (degreesToMeters(diff.norm()) > max_length) {  // TODO: use geoDistance?

			// split edge at center and mark as visited
			const auto center_vertex = mesh.split(eh, toVec3(center));
			mesh.mark(center_vertex);
            cnt++;
		}
    }
    std::cout << cnt << " splits" << std::endl;
}

void IsotropicRemeshing::collapseEdges(Mesh& mesh) const {
    int cnt = 0;
    for (const auto& eh : mesh.edges()) {

        if (eh.is_boundary()) {
            continue;
        }

        // incident points
		const vec2_t& p0 = toVec2(mesh.point(eh.v0()));
		const vec2_t& p1 = toVec2(mesh.point(eh.v1()));

		const real_t len = degreesToMeters((p1 - p0).norm());  // TODO: use geoDistance?

        const vec2_t center = (p0 + p1) / 2;
        const real_t min_length = min_size_factor * size.getValue(center);  // TODO: use maximum of p0 and p1?

        // check edge length
		if (len < min_length) {

            auto heh = eh.h0();
            if (heh.from().is_boundary()) {  // avoid collapsing vertices of the boundary
                heh = eh.h1();
            }

			// check if already visited
			if (!mesh.isMarked(eh.v0()) && !mesh.isMarked(eh.v1()) && mesh.is_collapse_ok(heh)) {

				mesh.collapse(heh);
                cnt++;
			}
        }
    }
    std::cout << cnt << " collapses" << std::endl;
}

void IsotropicRemeshing::equalizeValences(Mesh& mesh) const {
    int cnt = 0;
    for (const auto& eh : mesh.edges()) {

        if (eh.is_boundary()) {
            continue;
        }

		// get valence of relevant vertices
		const int vs = mesh.valence(eh.v0());
		const int vt = mesh.valence(eh.v1());
		const int vl = mesh.valence(mesh.opposite_vh(eh.h0()));
		const int vr = mesh.valence(mesh.opposite_vh(eh.h1()));

		// compute derivation from optimal valence
		const unsigned int e_old = std::abs(vs - 6) + std::abs(vt - 6) + std::abs(vl - 6) + std::abs(vr - 6);
		const unsigned int e_new = std::abs(vs - 7) + std::abs(vt - 7) + std::abs(vl - 5) + std::abs(vr - 5);

		if (e_new < e_old && mesh.is_flip_ok(eh)) {

			mesh.flip(eh);
			cnt++;
		}
	}
    std::cout << cnt << " flips" << std::endl;
}

void IsotropicRemeshing::smoothVertices(Mesh& mesh) const {

    for (const auto& vh : mesh.vertices()) {

        if (vh.is_boundary()) {
            continue;
        }

		// compute barycenter of one-ring
		vec2_t c(0);
		for (const auto& v : vh.vertices()) {
			c += toVec2(mesh.point(v));
		}

		c /= mesh.valence(vh);

		// update vertex position
		mesh.point(vh) = toVec3(c);
	}
}

}
