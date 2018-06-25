##############################
# Tests build description.
# Path: src/test
##############################

#OpenMP thread number environment variable.
set (ENV{OMP_NUM_THREADS} 8)

find_package( Boost COMPONENTS program_options REQUIRED )

add_subdirectory (gtest)

# Adds the executable
add_executable (Tests
	disk/ply_point_reader_test.cpp
	disk/heap_point_reader_test.cpp
	
	basic/array_test.cpp
	disk/point_sorter_test.cpp
	disk/ply_point_merger_test.cpp
	disk/ooc_point_sorter_test.cpp
	memory/tbb_allocator_test.cpp
# 	cpp/model/CameraTest.cpp
	renderer/frustum_test.cpp
	basic/point_test.cpp
	basic/morton_code_test.cpp
	basic/morton_interval_test.cpp
	hierarchy/o1_octree_node_test.cpp
	disk/serializer_test.cpp
	hierarchy/front_test.cpp
# 	cpp/model/FastParallelOctreeTest.cpp
	hierarchy/fast_parallel_octree_stress_test.cpp
	renderer/streaming_renderer_test.cpp
	renderer/splat_renderer_test.cpp
	disk/octree_file_test.cpp
	hierarchy/hierarchy_creator_no_render_test.cpp
	hierarchy/bvh_test.cpp
	renderer/mesh_test.cpp
	
	ui/text_test_widget.h                          ui/text_test_widget.cpp
	renderer/text_test.cpp
	ui/text_3d_test_widget.h                       ui/text_3d_test_widget.cpp
	renderer/text_3d_test.cpp
	hierarchy/fast_parallel_octree_test_param.h    hierarchy/fast_parallel_octree_test_param.cpp

	main.cpp)

target_include_directories( Tests
	PUBLIC
        Boost::program_options
		include
		../
)

target_link_libraries( Tests
	Point_Based_Renderer_Lib
	gtest
	gtest_main
	pthread
	Boost::program_options
)

# .ply files copy target.
add_custom_target( TestsCopy )
add_custom_command( TARGET TestsCopy PRE_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/data
	$<TARGET_FILE_DIR:Point_Based_Renderer_Lib>/test/data )
add_custom_command( TARGET TestsCopy PRE_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/StressTest.bash
	$<TARGET_FILE_DIR:Point_Based_Renderer_Lib>/test/StressTest.bash )
	
add_dependencies( Tests TestsCopy )
