#pragma once

#include <analysis/aggregates.h>
#include <analysis/assertions.h>
#include <analysis/grid_compare.h>
#include <analysis/mesh_quality.h>
#include <analysis/size_gradient.h>

#include <boundary/boundary_generator.h>
#include <boundary/boundary.h>

#include <geometry/line_graph.h>
#include <geometry/he_polygon.h>

#include <io/bin32_reader.h>
#include <io/csv_writer.h>
#ifdef OMG_REQUIRE_NETCDF
#include <io/nc_reader.h>
#endif  // OMG_REQUIRE_NETCDF
#include <io/nod2d_writer.h>
#include <io/off_writer.h>
#include <io/poly_reader.h>
#include <io/vtk_writer.h>

#include <mesh/mesh.h>
#include <mesh/remeshing.h>

#include <size_function/constant_size.h>
#include <size_function/gradient_limiting.h>
#include <size_function/reference_size.h>

#include <topology/scalar_field.h>

#include <triangulation/acute_triangulator.h>
#include <triangulation/jigsaw_triangulator.h>
#include <triangulation/triangle_triangulator.h>

#include <types.h>
#include <util.h>
