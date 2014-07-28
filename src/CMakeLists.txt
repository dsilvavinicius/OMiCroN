##############################
# Root build description.
# Path: src/
##############################

cmake_minimum_required (VERSION 2.8.11)
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
macro (create_source_groups ui_dir model_dir)
	# Creating file groups
	# Specifying sources.
	file (GLOB_RECURSE UI ${ui_dir}/*)
	file (GLOB_RECURSE MODEL ${model_dir}/*)
	# Creating source groups.
	source_group (ui FILES ${UI})
	source_group (model FILES ${MODEL})
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

create_source_groups (UI_DIR MODEL_DIR)

setup_package(OpenGL)
setup_package(GLM)

# Set Qt dirs.
find_package (Qt5Core REQUIRED)
find_package (Qt5Gui REQUIRED)
find_package (Qt5Widgets REQUIRED)

include_directories (${Qt5Core_INCLUDE_DIRS}
	${Qt5Gui_INCLUDE_DIRS}
	${Qt5Widgets_INCLUDE_DIRS}

	${HEADER_DIR}/${UI_DIR}/
	${HEADER_DIR}/${MODEL_DIR}/)

# Creates the renderer library.
add_library (Point_Based_Renderer_Lib
	${HEADER_DIR}/${UI_DIR}/OpenGLWindow.h			${CPP_DIR}/${UI_DIR}/OpenGLWindow.cpp
	${HEADER_DIR}/${UI_DIR}/PointRendererWindow.h	${CPP_DIR}/${UI_DIR}/PointRendererWindow.cpp
	
	${HEADER_DIR}/${MODEL_DIR}/Camera.h		${CPP_DIR}/${MODEL_DIR}/Camera.cpp
	${HEADER_DIR}/${MODEL_DIR}/MortonCode.h
	${HEADER_DIR}/${MODEL_DIR}/Octree.h		${CPP_DIR}/${MODEL_DIR}/Octree.cpp
	${HEADER_DIR}/${MODEL_DIR}/OctreeNode.h
	${HEADER_DIR}/${MODEL_DIR}/InnerNode.h	${CPP_DIR}/${MODEL_DIR}/InnerNode.cpp
	${HEADER_DIR}/${MODEL_DIR}/LeafNode.h	${CPP_DIR}/${MODEL_DIR}/LeafNode.cpp
	${HEADER_DIR}/${MODEL_DIR}/Point.h		${CPP_DIR}/${MODEL_DIR}/Point.cpp)

# Qt 5 modules.
target_link_libraries (Point_Based_Renderer_Lib Qt5::Core
	Qt5::Gui
	Qt5::Widgets
	${OPENGL_LIBRARIES}
	${GLML_LIBRARIES})
	
# Creates renderer program and links with the renderer library.
add_executable (Point_Based_Renderer
	${CPP_DIR}/main.cpp
	./CMakeLists.txt)
target_include_directories (Point_Based_Renderer PUBLIC Point_Based_Renderer_Lib)
target_link_libraries (Point_Based_Renderer Point_Based_Renderer_Lib)
	
# Tests directory.
add_subdirectory (test)