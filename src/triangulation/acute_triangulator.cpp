
#include "acute_triangulator.h"

#include <triangle_api.h>
#include <triangulation/triangle_helper.h>

namespace omg {

const SizeFunction* ACuteTriangulator::size_function;

ACuteTriangulator::ACuteTriangulator() {
    ctx = triangle_context_create();

    behavior options;
    triangle_context_get_behavior(ctx, &options);

    // zBPupDq25
    options.firstnumber = 0;
    options.nobound = true;
    // -P does not exist
    options.usertest = true;
    options.triunsuitable_user_func = triunsuitable;
    options.poly = true;
    options.conformdel = true;
    options.quality = true;
    options.minangle = 25.0f;

    check(triangle_context_set_behavior(ctx, &options));
}

ACuteTriangulator::~ACuteTriangulator() {
    triangle_context_destroy(ctx);
}

void ACuteTriangulator::generateMesh(const Polygon& outline, const SizeFunction& size, Mesh& out_mesh) {
    restrictToInt(outline);

    size_function = &size;

    TriangleIn<triangleio> in(outline);
    TriangleOut<triangleio> out;

    check(triangle_mesh_create(ctx, &in.io));

    check(triangle_mesh_copy(ctx, &out.io, false, false));

    out.toMesh(out_mesh);

    size_function = nullptr;
}

int ACuteTriangulator::triunsuitable(double* v1, double* v2, double* v3, double area) {
    (void) area;  // unused

    if (size_function == nullptr) {
        throw std::runtime_error("size_function was null");
    }

    return !size_function->isTriangleGood(vec2_t(v1[0], v1[1]), vec2_t(v2[0], v2[1]), vec2_t(v3[0], v3[1]));
}

void ACuteTriangulator::check(int status_code) const {
    std::string msg;
    switch (status_code) {
        case TRI_OK:
            return;
        case TRI_FAILURE:
            msg = "Triangulation failure";
            break;
        case TRI_OPTIONS:
            msg = "Invalid options";
            break;
        case TRI_INPUT:
            msg = "Invalid input";
            break;
        case TRI_SEG_SPLIT:
            msg = "Internal triangulation error or not enough precision";
            break;
        case TRI_FIND_DIRECTION:
        case TRI_SEG_INTERSECT:
        case TRI_SEG_INSERT:
        case TRI_SEG_SCOUT:
            msg = "Internal triangulation error";
            break;
        case TRI_FILE_OPEN:
            msg = "Error opening file";
            break;
        case TRI_FILE_READ:
            msg = "Error reading file";
            break;
        case TRI_NULL:
            msg = "Argument was NULL";
            break;
    }
    throw std::runtime_error(msg);
}

}
