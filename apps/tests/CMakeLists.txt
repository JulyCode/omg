
add_executable(MedseaTest medsea_test.cpp)
target_link_libraries(MedseaTest PRIVATE OMG)

add_executable(RegionTest region_test.cpp)
target_link_libraries(RegionTest PRIVATE OMG)

add_executable(CreateStats create_stats.cpp)
target_link_libraries(CreateStats PRIVATE OMG)

add_executable(LimitingTest limiting_test.cpp)
target_link_libraries(LimitingTest PRIVATE OMG)