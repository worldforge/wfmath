# Add test
wf_add_test(ball_test.cpp)
wf_add_test(const_test.cpp)
wf_add_test(intstring_test.cpp)
wf_add_test(line_test.cpp)
wf_add_test(point_test.cpp)
wf_add_test(polygon_test.cpp)
wf_add_test(probability_test.cpp)
wf_add_test(quaternion_test.cpp)
wf_add_test(randgen_test.cpp)
wf_add_test(rotmatrix_test.cpp)
wf_add_test(shape_test.cpp)
wf_add_test(timestamp_test.cpp)
wf_add_test(vector_test.cpp)

if (Atlas_FOUND)
    link_directories(${Atlas_LIBRARY_DIR})
    wf_add_test(atlas/atlas_0_7_test.cpp)
    target_link_libraries(atlas_0_7_test ${Atlas_LIBRARIES})
    target_include_directories(atlas_0_7_test PUBLIC ${Atlas_INCLUDE_DIR})
endif (Atlas_FOUND)