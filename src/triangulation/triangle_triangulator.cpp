
#include "triangle_triangulator.h"

#include <Triangle/jrs_triangle.h>
#include <triangulation/triangle_helper.h>
#include <util.h>

namespace omg {

const SizeFunction* TriangleTriangulator::size_function;

TriangleTriangulator::TriangleTriangulator() {
    // init callback function to interact with Triangle
    jrs::set_triunsuitable_callback(triunsuitable);
}

void TriangleTriangulator::generateMesh(const Boundary& boundary, const SizeFunction& size, Mesh& out_mesh) {
    size_function = &size;

    ScopeTimer timer("Triangle generate mesh");

    TriangleIn<jrs::triangulateio> in(boundary);
    TriangleOut<jrs::triangulateio> out;

    // Q: no console output
    // z: use zero based indexing
    // B: don't output boundary information
    // P: don't output segment information
    // u: use triunsuitable quality check
    // p: triangulate a Planar Straight Line Graph (.poly file)
    // D: conforming Delaunay
    // q25: minimum angle of 25 degrees, maximum 180 - 2 * 25
    TriangleArgs args;
    args.quiet = false;
    args.zero_based = true;
    args.nobound = true;
    args.nopolywritten = true;
    args.user_test = true;
    args.input_type = TriangleArgs::InputType::POLY;
    args.conformdel = true;
    args.quality = true;
    args.min_angle = 25;

    jrs::triangulate(args.toString(), &in.io, &out.io, nullptr);

    out.toMesh(out_mesh);

    size_function = nullptr;
}

int TriangleTriangulator::triunsuitable(double* v1, double* v2, double* v3, double area) {
    (void) area;  // unused

    if (size_function == nullptr) {
        throw std::runtime_error("size_function was null");
    }

    return !size_function->isTriangleGood(vec2_t(v1[0], v1[1]), vec2_t(v2[0], v2[1]), vec2_t(v3[0], v3[1]));
}


TriangleArgs::TriangleArgs() : args(nullptr) {
    input_type = InputType::POLY;
    quality = false;
    min_angle = 0;
    area_constraint = AreaConstraint::NONE;
    max_area = 0;
    user_test = false;
    region_attrib = false;
    convex = false;
    weighted = Weights::NONE;
    jettison = false;
    zero_based = true;
    neighbors = false;
    nobound = false;
    noholes = false;
    noexact = false;
    conformdel = false;
    incremental = false;
    sweepline = false;
    dwyer = true;
    splitseg = false;
    nobisect = false;
    limit_steiner = false;
    steiner = 0;
    quiet = false;
    verbose = false;
    nopolywritten = false;
}

TriangleArgs::~TriangleArgs() {
    if (args != nullptr) {
        delete[] args;
    }
}

char* TriangleArgs::toString() {
    if (args != nullptr) {
        throw std::runtime_error("toString can only be used once");
    }

    std::string s;

    // build argument string
    if (input_type == InputType::POLY) {
        s += "p";
    } else if (input_type == InputType::MESH) {
        s += "r";
    }

    if (quality) {
        s += "q" + std::to_string(min_angle);
    }

    if (area_constraint == AreaConstraint::FILE) {
        s += "a";
    } else if (area_constraint == AreaConstraint::CUSTOM) {
        s += "a" + std::to_string(max_area);
    }

    if (user_test) { s += "u"; }

    if (region_attrib) { s += "A"; }

    if (convex) {  s += "c"; }

    if (weighted == Weights::MODE_1) {
        s += "w";
    } else if (weighted == Weights::MODE_2) {
        s += "W";
    }

    if (jettison) { s += "j"; }

    if (zero_based) { s += "z"; }

    if (neighbors) { s += "n"; }

    if (nobound) { s += "B"; }

    if (noholes) { s += "O"; }

    if (noexact) { s += "X"; }

    if (conformdel) { s += "D"; }

    if (incremental) { s += "i"; }

    if (sweepline) { s += "F"; }

    if (!dwyer) { s += "l"; }

    if (splitseg) { s += "s"; }

    if (nobisect) { s += "Y"; }

    if (limit_steiner) {
        s += "S" + std::to_string(steiner);
    }

    if (quiet) { s += "Q"; }

    if (verbose) { s += "V"; }

    if (nopolywritten) { s += "P"; }

    // copy arguments to new char[]
    args = new char[s.size() + 1];
    for (std::size_t i = 0; i < s.size(); i++) {
        args[i] = s[i];
    }
    args[s.size()] = '\0';

    return args;
}

}
