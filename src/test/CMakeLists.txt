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
	${CPP_DIR}/${MODEL_DIR}/CameraTest.cpp
	${CPP_DIR}/${MODEL_DIR}/FrustumTest.cpp
	${CPP_DIR}/${MODEL_DIR}/PointTest.cpp
	${CPP_DIR}/${MODEL_DIR}/MortonCodeTest.cpp
	${CPP_DIR}/${MODEL_DIR}/OctreeTest.cpp
	${CPP_DIR}/${MODEL_DIR}/RandomSampleOctreeTest.cpp
	${CPP_DIR}/${MODEL_DIR}/OutOfCoreOctreeTest.cpp
	${CPP_DIR}/${MODEL_DIR}/FrontTest.cpp
	${HEADER_DIR}/${MODEL_DIR}/ScanQGLView.h		${CPP_DIR}/${MODEL_DIR}/ScanQGLView.cpp
	${CPP_DIR}/${MODEL_DIR}/ScanTest.cpp
	${HEADER_DIR}/${MODEL_DIR}/CompactionQGLView.h	${CPP_DIR}/${MODEL_DIR}/CompactionQGLView.cpp
	${CPP_DIR}/${MODEL_DIR}/CompactionTest.cpp
	
	${CPP_DIR}/${UTIL_DIR}/PlyPointReaderTest.cpp
	${CPP_DIR}/main.cpp)

target_include_directories (Tests PUBLIC include/)
target_include_directories (Tests PUBLIC gtest
	gtest_main)
target_link_libraries (Tests Point_Based_Renderer_Lib
	gtest
	gtest_main
	pthread)