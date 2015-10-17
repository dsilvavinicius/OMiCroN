##############################
# Tests build description.
# Path: src/test
##############################

create_source_groups (UI_DIR MODEL_DIR UTIL_DIR)

include_directories( include/
	${HEADER_DIR}/${MODEL_DIR}/ )

add_subdirectory (gtest/)

# Adds the executable
add_executable (Tests
	${CPP_DIR}/${UTIL_DIR}/PlyPointReaderTest.cpp
	
	${CPP_DIR}/${MODEL_DIR}/HierarchyTestMethods.cpp
	${CPP_DIR}/${MODEL_DIR}/Ken12MemoryManagerTest.cpp
	${CPP_DIR}/${MODEL_DIR}/MemoryManagerTest.cpp
	${CPP_DIR}/${MODEL_DIR}/CameraTest.cpp
	${CPP_DIR}/${MODEL_DIR}/FrustumTest.cpp
	${CPP_DIR}/${MODEL_DIR}/PointTest.cpp
	${CPP_DIR}/${MODEL_DIR}/MortonCodeTest.cpp
	${CPP_DIR}/${MODEL_DIR}/MortonIntervalTest.cpp
	${CPP_DIR}/${MODEL_DIR}/OctreeNodeTest.cpp
	${CPP_DIR}/${MODEL_DIR}/SerializerTest.cpp
	${CPP_DIR}/${MODEL_DIR}/SQLiteManagerTest.cpp
	${CPP_DIR}/${MODEL_DIR}/OctreeTest.cpp
	${CPP_DIR}/${MODEL_DIR}/RandomSampleOctreeTest.cpp
	${CPP_DIR}/${MODEL_DIR}/OutOfCoreOctreeTest.cpp
	
	${HEADER_DIR}/${MODEL_DIR}/TextTestWidget.h		${CPP_DIR}/${MODEL_DIR}/TextTestWidget.cpp
	${CPP_DIR}/${MODEL_DIR}/TextTest.cpp
	${HEADER_DIR}/${MODEL_DIR}/Text3DTestWidget.h		${CPP_DIR}/${MODEL_DIR}/Text3DTestWidget.cpp
	${CPP_DIR}/${MODEL_DIR}/Text3DTest.cpp

#	The tests below are not compatible with glew and Tucano, since are using new Qt only.

#	${HEADER_DIR}/${MODEL_DIR}/ScanQGLView.h		${CPP_DIR}/${MODEL_DIR}/ScanQGLView.cpp
#	${CPP_DIR}/${MODEL_DIR}/ScanTest.cpp
#	${HEADER_DIR}/${MODEL_DIR}/CompactionQGLView.h	${CPP_DIR}/${MODEL_DIR}/CompactionQGLView.cpp
#	${CPP_DIR}/${MODEL_DIR}/CompactionTest.cpp
	
	${CPP_DIR}/main.cpp)

target_include_directories (Tests PUBLIC include/)
target_include_directories (Tests PUBLIC gtest
	gtest_main)
target_link_libraries (Tests Point_Based_Renderer_Lib
	gtest
	gtest_main
	pthread)

# .ply files copy target.
add_custom_target( TestsCopy )
get_target_property( Renderer_Location Point_Based_Renderer LOCATION )
get_filename_component( Renderer_Dir ${Renderer_Location} PATH)
add_custom_command( TARGET TestsCopy PRE_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/data
	${Renderer_Dir}/test/data )
	
add_dependencies( Tests TestsCopy )