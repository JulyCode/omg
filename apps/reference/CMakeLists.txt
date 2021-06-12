
add_executable(Reference_basic triangle.c)

target_link_libraries(Reference_basic PRIVATE m)

target_compile_definitions(Reference_basic  PRIVATE WORKING_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/"
                                            PRIVATE INPUT_FILE="${CMAKE_CURRENT_SOURCE_DIR}/meshes/medsea.poly"
                                            PRIVATE SWITCHES="-pq25Da0.5V")

add_executable(Reference_refine triangle_refine_proj_bin.c)

target_link_libraries(Reference_refine PRIVATE m)

target_compile_definitions(Reference_refine PRIVATE WORKING_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/"
                                            PRIVATE INPUT_FILE="${CMAKE_CURRENT_SOURCE_DIR}/meshes/medsea.1"
                                            PRIVATE SWITCHES="-rupq30DV")
