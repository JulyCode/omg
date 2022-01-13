
#include "remeshing.h"

#include <iostream>
#include <unordered_map>

#include <util.h>

namespace omg {

IsotropicRemeshing::IsotropicRemeshing(const SizeFunction& size, real_t min_size_factor, real_t max_size_factor)
    : size(size), min_size_factor(min_size_factor), max_size_factor(max_size_factor) {}

void IsotropicRemeshing::remesh(Mesh& mesh, unsigned int iterations) const {
    ScopeTimer timer("Isotropic remeshing");

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

        // incident points
		const vec2_t& p0 = toVec2(mesh.point(eh.v0()));
		const vec2_t& p1 = toVec2(mesh.point(eh.v1()));

		const vec2_t diff = p1 - p0;

        const vec2_t center = p0 + diff / 2;
        const real_t max_length = max_size_factor * size.getValue(center);

        // check edge length
		if (diff.norm() > max_length) {  // TODO: use geoDistance?

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

        // incident points
		const vec2_t& p0 = toVec2(mesh.point(eh.v0()));
		const vec2_t& p1 = toVec2(mesh.point(eh.v1()));

		const real_t len = (p1 - p0).norm();  // TODO: use geoDistance?

        const vec2_t center = (p0 + p1) / 2;
        const real_t min_length = min_size_factor * size.getValue(center);

        // check edge length
		if (len < min_length) {

            auto heh = eh.h0();  // halfedge to collapse, controlls which vertex is deleted

            // test if the edge has a parallel neighbor edge on the boundary
            if (eh.is_boundary()) {

                const auto& boundary_heh = eh.h0().is_boundary() ? eh.h0() : eh.h1();

                if (isCollinear(mesh, boundary_heh, boundary_heh.prev().from())) {
                    heh = boundary_heh;
                } else if (isCollinear(mesh, boundary_heh, boundary_heh.next().to())) {
                    heh = boundary_heh.opp();
                } else {
                    continue;
                }

            } else {

                if (heh.from().is_boundary()) {  // avoid collapsing vertices of the boundary
                    heh = eh.h1();
                }
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

bool IsotropicRemeshing::isCollinear(const Mesh& mesh, const OpenMesh::SmartHalfedgeHandle& heh,
                                     const OpenMesh::SmartVertexHandle& vh) const {

    const vec2_t p0 = toVec2(mesh.point(heh.from()));
    const vec2_t p1 = toVec2(mesh.point(heh.to()));
    const vec2_t p2 = toVec2(mesh.point(vh));

    const real_t len0 = (p1 - p0).norm();
    const real_t len1 = (p2 - p0).norm();
    const real_t len2 = (p2 - p1).norm();

    const real_t max_len = std::max({len0, len1, len2});

    // for three points a, b ,c: check if ab + bc = ac
    return std::abs(len0 + len1 + len2 - 2 * max_len) < max_len * 0.000001;
}

void IsotropicRemeshing::equalizeValences(Mesh& mesh) const {
    int cnt = 0;
    for (const auto& eh : mesh.edges()) {

        if (eh.is_boundary()) {
            continue;
        }

        // get vertices
        const auto& s = eh.v0();
        const auto& t = eh.v1();
        const auto& l = eh.h0().next().to();
        const auto& r = eh.h1().next().to();

		// get valence of relevant vertices
		const int vs = mesh.valence(s);
		const int vt = mesh.valence(t);
		const int vl = mesh.valence(l);
		const int vr = mesh.valence(r);

        // get optimal valences
        const int target_vs = computeOptimalValence(s, mesh);
        const int target_vt = computeOptimalValence(t, mesh);
        const int target_vl = computeOptimalValence(l, mesh);
        const int target_vr = computeOptimalValence(r, mesh);

        // compute derivation from optimal valence
        const int d_vs = vs - target_vs;
        const int d_vt = vt - target_vt;
        const int d_vl = vl - target_vl;
        const int d_vr = vr - target_vr;

		unsigned int e_old = d_vs * d_vs + d_vt * d_vt + d_vl * d_vl + d_vr * d_vr;

		unsigned int e_new = (d_vs - 1) * (d_vs - 1) + (d_vt - 1) * (d_vt - 1);
        e_new +=             (d_vl + 1) * (d_vl + 1) + (d_vr + 1) * (d_vr + 1);

		if (e_new < e_old && mesh.is_flip_ok(eh)) {

			mesh.flip(eh);
			cnt++;
		}
	}
    std::cout << cnt << " flips" << std::endl;
}

int IsotropicRemeshing::computeOptimalValence(const OpenMesh::SmartVertexHandle& vh, const Mesh& mesh) {
    if (!vh.is_boundary()) {
        return 6;
    }

    if (!mesh.is_valid_handle(vh.halfedge())) {
        return 0;
    }

    const vec2_t& point = toVec2(mesh.point(vh));

    // get the neighbors on the boundary
    const auto& a = vh.halfedge().prev().from();
    const auto& b = vh.halfedge().to();

    assert(a.is_boundary() && b.is_boundary() && vh.halfedge().is_boundary());

    // vectors to a and b
    const vec2_t d1 = (toVec2(mesh.point(a)) - point).normalized();
    const vec2_t d2 = (toVec2(mesh.point(b)) - point).normalized();

    // compute angle fraction
    real_t angle = std::acos(d1.dot(d2)) / (2 * PI);

    // check if bigger angle should be used
    if (d1[0] * d2[1] - d1[1] * d2[0] < 0) {  // z component of cross product
        angle = 1 - angle;
    }

    // valence depending on angle, but at least 2
    return std::clamp<int>(std::lround(angle * 6) + 1, 2, 6);
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
