##############################
# Root build description.
# Path: src/
##############################

cmake_minimum_required (VERSION 2.8.11)
project (Point_Based_Renderer)

macro (setup_package package)
	find_package (${package} REQUIRED)
	include_directories (${${package}_INCLUDE_DIRS})
	link_directories(${${package}_LIBRARY_DIRS})
	add_definitions(${${package}_DEFINITIONS})
	
	if(NOT ${package}_FOUND)
		message(Error "${package} not found")
	endif(NOT ${package}_FOUND)
endmacro (setup_package)

# Adding package finders directory.
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Find includes in corresponding build directories.
set (CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set (CMAKE_AUTOMOC ON)
# Necessary flags for C++11 and linking to Qt5.
SET (CMAKE_CXX_FLAGS "-std=c++11 -fPIC")

set (UI_DIR "ui")
set (MODEL_DIR "model")

set (HEADER_DIR "header")
set (CPP_DIR "src")

setup_package(OpenGL)
setup_package(GLM)

# Set Qt dirs.
find_package (Qt5Core REQUIRED)
find_package (Qt5Gui REQUIRED)
find_package (Qt5Widgets REQUIRED)

include_directories (${Qt5Core_INCLUDE_DIRS}
	${Qt5Gui_INCLUDE_DIRS}
	${Qt5Widgets_INCLUDE_DIRS}

	${UI_DIR}/${HEADER_DIR}/
	${MODEL_DIR}/${HEADER_DIR}/)

# Creating file groups
# Specifying sources.
file (GLOB_RECURSE UI ${UI_DIR}/*)
file (GLOB_RECURSE MODEL ${MODEL_DIR}/*)
# Creating source groups.
source_group (ui FILES ${UI})
source_group (model FILES ${MODEL})

# Add the executable
add_executable (Point_Based_Renderer
	${UI_DIR}/${HEADER_DIR}/OpenGLWindow.h			${UI_DIR}/${CPP_DIR}/OpenGLWindow.cpp
	${UI_DIR}/${HEADER_DIR}/PointRendererWindow.h	${UI_DIR}/${CPP_DIR}/PointRendererWindow.cpp
	
	${MODEL_DIR}/${HEADER_DIR}/Camera.h		${MODEL_DIR}/${CPP_DIR}/Camera.cpp
	./main.cpp
	./CMakeLists.txt)

# Qt 5 modules.
target_link_libraries (Point_Based_Renderer Qt5::Core
	Qt5::Gui
	Qt5::Widgets
	${OPENGL_LIBRARIES}
	${GLML_LIBRARIES})