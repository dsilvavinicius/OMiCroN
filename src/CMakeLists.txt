##############################
# Root build description.
# Path: src/
##############################

cmake_minimum_required (VERSION 2.8.12)
project (Point_Based_Renderer)

# Setups the given package, trying to find it and sets includes and linking directories.
macro (setup_package package)
	find_package (${package} REQUIRED)
	include_directories (${${package}_INCLUDE_DIRS})
	link_directories(${${package}_LIBRARY_DIRS})
	add_definitions(${${package}_DEFINITIONS})
	
	if(NOT ${package}_FOUND)
		message(Error "${package} not found")
	endif(NOT ${package}_FOUND)
endmacro (setup_package)

# Creates the same source groups for library and tests.
macro (create_source_groups ui_dir model_dir util_dir)
	# Creating file groups
	# Specifying sources.
	file (GLOB_RECURSE UI ${ui_dir}/*)
	file (GLOB_RECURSE MODEL ${model_dir}/*)
	file (GLOB_RECURSE UTIL ${util_dir}/*)
	# Creating source groups.
	source_group (ui FILES ${UI})
	source_group (model FILES ${MODEL})
	source_group (util FILES ${UTIL})
endmacro (create_source_groups)

# Adding package finders directory.
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Find includes in corresponding build directories.
set (CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set (CMAKE_AUTOMOC ON)
# Necessary flags for C++11 and linking to Qt5.
SET (CMAKE_CXX_FLAGS "-std=c++11 -fPIC")

set (HEADER_DIR "header")
set (CPP_DIR "cpp")

set (UI_DIR "ui")
set (MODEL_DIR "model")
set (UTIL_DIR "util")

create_source_groups (UI_DIR MODEL_DIR UTIL_DIR)

setup_package(OpenGL)
setup_package(GLM)

# Set Qt dirs.
find_package (Qt5Core REQUIRED)
find_package (Qt5Gui REQUIRED)
find_package (Qt5Widgets REQUIRED)
find_package (Qt53D REQUIRED)

include_directories (${Qt5Core_INCLUDE_DIRS}
	${Qt5Gui_INCLUDE_DIRS}
	${Qt5Widgets_INCLUDE_DIRS}
	${Qt53D_INCLUDE_DIRS}
	include/
	
	${HEADER_DIR}/${UI_DIR}/
	${HEADER_DIR}/${MODEL_DIR}/
	${HEADER_DIR}/${UTIL_DIR}/)

# Compiles libraries
add_subdirectory(lib)
	
# Creates the renderer library.
add_library (Point_Based_Renderer_Lib
	${HEADER_DIR}/${UI_DIR}/PointRendererWindow.h	${CPP_DIR}/${UI_DIR}/PointRendererWindow.cpp
	
	${HEADER_DIR}/${MODEL_DIR}/RenderingState.h
	${HEADER_DIR}/${MODEL_DIR}/Camera.h				${CPP_DIR}/${MODEL_DIR}/Camera.cpp
	${HEADER_DIR}/${MODEL_DIR}/MortonCode.h			${CPP_DIR}/${MODEL_DIR}/MortonCode.cpp
	${HEADER_DIR}/${MODEL_DIR}/RandomSampleOctree.h
	${HEADER_DIR}/${MODEL_DIR}/OctreeStats.h
	${HEADER_DIR}/${MODEL_DIR}/Octree.h
	${HEADER_DIR}/${MODEL_DIR}/OctreeNode.h
	${HEADER_DIR}/${MODEL_DIR}/InnerNode.h
	${HEADER_DIR}/${MODEL_DIR}/LeafNode.h
	${HEADER_DIR}/${MODEL_DIR}/Point.h
	${HEADER_DIR}/${MODEL_DIR}/ExtendedPoint.h
	
	${HEADER_DIR}/${UTIL_DIR}/Stream.h			${CPP_DIR}/${UTIL_DIR}/Stream.cpp
	${HEADER_DIR}/${UTIL_DIR}/PlyPointReader.h)

# Qt 5 modules.
target_link_libraries (Point_Based_Renderer_Lib Qt5::Core
	Qt5::Gui
	Qt5::Widgets
	Qt5::3D
	${OPENGL_LIBRARIES}
	${GLML_LIBRARIES}
	RPly)

# Creates renderer program and links with the renderer library.
add_executable (Point_Based_Renderer
	${CPP_DIR}/main.cpp
	./CMakeLists.txt)
target_include_directories (Point_Based_Renderer PUBLIC Point_Based_Renderer_Lib)
target_link_libraries (Point_Based_Renderer Point_Based_Renderer_Lib)
	
# Tests directory.
add_subdirectory (test)