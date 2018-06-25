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
	cpp/util/PlyPointReaderTest.cpp
	cpp/util/HeapPointReaderTest.cpp
	
	cpp/model/ArrayTest.cpp
	cpp/model/PointSorterTest.cpp
	cpp/model/PlyPointMergerTest.cpp
	cpp/model/OocPointSorterTest.cpp
	cpp/model/TbbAllocatorTest.cpp
	cpp/model/CameraTest.cpp
	cpp/model/FrustumTest.cpp
	cpp/model/PointTest.cpp
	cpp/model/MortonCodeTest.cpp
	cpp/model/MortonIntervalTest.cpp
	cpp/model/O1OctreeNodeTest.cpp
	cpp/model/SerializerTest.cpp
	cpp/model/FrontTest.cpp
# 	cpp/model/FastParallelOctreeTest.cpp
	cpp/model/FastParallelOctreeStressTest.cpp
	cpp/model/StreamingRendererTest.cpp
	cpp/model/SplatRendererTest.cpp
	cpp/model/OctreeFileTest.cpp
	cpp/model/HierarchyCreatorNoRenderTest.cpp
	cpp/model/BvhTest.cpp
	cpp/tucano/MeshTest.cpp
	
	header/model/TextTestWidget.h          cpp/model/TextTestWidget.cpp
	cpp/model/TextTest.cpp
	header/model/Text3DTestWidget.h        cpp/model/Text3DTestWidget.cpp
	cpp/model/Text3DTest.cpp
	header/FastParallelOctreeTestParam.h   cpp/FastParallelOctreeTestParam.cpp

	cpp/main.cpp)

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
