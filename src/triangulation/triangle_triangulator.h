#pragma once

#include <triangulation/triangulator.h>
#include <types.h>

namespace omg {

class TriangleTriangulator : public Triangulator {
public:
    explicit TriangleTriangulator(real_t min_angle = 33);

    void generateMesh(const Boundary& boundary, const SizeFunction& size, Mesh& out_mesh) override;

private:
    static const SizeFunction* size_function;

    static int triunsuitable(double* v1, double* v2, double* v3, double area);

    const real_t min_angle;
};


// not all switches are supported
struct TriangleArgs {
public:
    TriangleArgs();
    ~TriangleArgs();

    enum class InputType {
        POLY, MESH
    };

    enum class AreaConstraint {
        NONE, FILE, CUSTOM
    };

    enum class Weights {
        NONE, MODE_1, MODE_2
    };

    /* Triangulate a Planar Straight Line Graph (-p switch) */
    /* or refine a previously generated mesh (-r switch). */
    InputType input_type;

    /* Quality mesh generation (-q switch). */
    /* Minimum angle bound (specified after -q switch). */
    bool quality;
    real_t min_angle;

    /* Apply an area constraint per triangle (-a switch without number). */
    /* or apply a global maximum triangle area constraint (-a switch with number). */
    AreaConstraint area_constraint;
    real_t max_area;

    /* Apply a user-defined triangle constraint (-u switch). */
    bool user_test;

    /* Apply attributes to identify triangles in certain regions. (-A switch). */
    bool region_attrib;

    /* Enclose the convex hull with segments. (-c switch). */
    bool convex;

    /* Weighted Delaunay triangulation (1 for -w switch) or regular triangulation */
    /* ie. lower hull of a height field (2 for -W switch). */
    Weights weighted;

    /* Jettison unused vertices from output (-j switch). */
    bool jettison;

    /* Number all items starting from zero or one (0 for -z switch). */
    bool zero_based;

    /* Generate a list of triangle neighbors (-n switch). */
    bool neighbors;

    /* Suppress output of boundary information (-B switch). */
    bool nobound;

    /* Ignore holes (-O switch). */
    bool noholes;

    /* Suppress use of exact arithmetic (-X switch). */
    bool noexact;

    /* Conforming Delaunay: all triangles are truly Delaunay (-D switch). */
    bool conformdel;

    /* Use incremental method (-i switch). */
    bool incremental;

    /* Use Fortune's sweepline algorithm (-F switch). */
    bool sweepline;

    /* Use alternating cuts for divide-and-conquer (inverse of -l switch). */
    bool dwyer;

    /* Force segments into mesh by splitting instead of using CDT (-s switch). */
    bool splitseg;

    /* Suppress boundary segment splitting (-Y switch). */
    bool nobisect;

    /* Maximum number of added Steiner points (specified after -S switch).   */
    bool limit_steiner;
    int steiner;

    bool quiet;
    bool verbose;

    bool nopolywritten;

    char* toString();

private:
    char* args;
};

}
