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
	${CPP_DIR}/${UTIL_DIR}/PlyPointReaderTest.cpp
	${CPP_DIR}/${UTIL_DIR}/HeapPointReaderTest.cpp
	
	${CPP_DIR}/${MODEL_DIR}/ArrayTest.cpp
	${CPP_DIR}/${MODEL_DIR}/PointSorterTest.cpp
	${CPP_DIR}/${MODEL_DIR}/PlyPointMergerTest.cpp
	${CPP_DIR}/${MODEL_DIR}/OocPointSorterTest.cpp
	${CPP_DIR}/${MODEL_DIR}/TbbAllocatorTest.cpp
	${CPP_DIR}/${MODEL_DIR}/CameraTest.cpp
	${CPP_DIR}/${MODEL_DIR}/FrustumTest.cpp
	${CPP_DIR}/${MODEL_DIR}/PointTest.cpp
	${CPP_DIR}/${MODEL_DIR}/MortonCodeTest.cpp
	${CPP_DIR}/${MODEL_DIR}/MortonIntervalTest.cpp
	${CPP_DIR}/${MODEL_DIR}/O1OctreeNodeTest.cpp
	${CPP_DIR}/${MODEL_DIR}/SerializerTest.cpp
	${CPP_DIR}/${MODEL_DIR}/FrontTest.cpp
	${CPP_DIR}/${MODEL_DIR}/FastParallelOctreeTest.cpp
	${CPP_DIR}/${MODEL_DIR}/FastParallelOctreeStressTest.cpp
	${CPP_DIR}/${MODEL_DIR}/StreamingRendererTest.cpp
	${CPP_DIR}/${MODEL_DIR}/SplatRendererTest.cpp
	${CPP_DIR}/${MODEL_DIR}/OctreeFileTest.cpp
	${CPP_DIR}/${MODEL_DIR}/HierarchyCreatorNoRenderTest.cpp
	${CPP_DIR}/${MODEL_DIR}/BvhTest.cpp
	${CPP_DIR}/tucano/MeshTest.cpp
	
	${HEADER_DIR}/${MODEL_DIR}/TextTestWidget.h		${CPP_DIR}/${MODEL_DIR}/TextTestWidget.cpp
	${CPP_DIR}/${MODEL_DIR}/TextTest.cpp
	${HEADER_DIR}/${MODEL_DIR}/Text3DTestWidget.h	${CPP_DIR}/${MODEL_DIR}/Text3DTestWidget.cpp
	${CPP_DIR}/${MODEL_DIR}/Text3DTest.cpp
	${HEADER_DIR}/FastParallelOctreeTestParam.h	${CPP_DIR}/FastParallelOctreeTestParam.cpp

	${CPP_DIR}/main.cpp)

target_include_directories( Tests
	PUBLIC
        Boost::program_options
		include
		${HEADER_DIR}/
		${HEADER_DIR}/${MODEL_DIR}/
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
