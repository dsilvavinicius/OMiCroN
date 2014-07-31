##############################
# Tests build description.
# Path: src/test
##############################

create_source_groups (UI_DIR MODEL_DIR)

add_subdirectory (gtest/)

# Adds the executable
add_executable (Tests
	${CPP_DIR}/${MODEL_DIR}/CameraTest.cpp
	${CPP_DIR}/${MODEL_DIR}/MortonCodeTest.cpp
	${CPP_DIR}/${MODEL_DIR}/OctreeTest.cpp
	${CPP_DIR}/main.cpp)

target_include_directories (Tests PUBLIC include/)
target_include_directories (Tests PUBLIC gtest
	gtest_main)
target_link_libraries (Tests Point_Based_Renderer_Lib
	gtest
	gtest_main
	pthread)