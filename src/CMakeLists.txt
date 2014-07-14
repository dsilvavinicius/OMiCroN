##############################
# Root build description.
# Path: src/
##############################

cmake_minimum_required (VERSION 2.8.11)
project (Point_Based_Renderer)

# Find includes in corresponding build directories
set (CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set (CMAKE_AUTOMOC ON)
# Necessary flags for C++11 and linking to Qt5.
SET (CMAKE_CXX_FLAGS "-std=c++11 -fPIC")

set (UI_DIR "ui")

set (HEADER_DIR "header")
set (CPP_DIR "src")

# Set OpenGL dirs.
find_package (OpenGL REQUIRED)
include_directories (${OPENGL_INCLUDE_DIRS})
link_directories(${OpenGL_LIBRARY_DIRS})
add_definitions(${OpenGL_DEFINITIONS})

# Set Qt dirs.
find_package (Qt5Core REQUIRED)
find_package (Qt5Gui REQUIRED)
find_package (Qt5Widgets REQUIRED)

include_directories (${Qt5Core_INCLUDE_DIRS})
include_directories (${Qt5Gui_INCLUDE_DIRS})
include_directories (${Qt5Widgets_INCLUDE_DIRS})

include_directories (${UI_DIR}/${HEADER_DIR}/)

# Creating file groups
# Specifying sources.
file (GLOB_RECURSE UI ${UI_DIR}/*)
# Creating source groups.
source_group (ui FILES ${UI})

# Add the executable
add_executable (Point_Based_Renderer
	${UI_DIR}/${HEADER_DIR}/OpenGLWindow.h			${UI_DIR}/${CPP_DIR}/OpenGLWindow.cpp
	${UI_DIR}/${HEADER_DIR}/PointRendererWindow.h	${UI_DIR}/${CPP_DIR}/PointRendererWindow.cpp
	./main.cpp
	./CMakeLists.txt)

# Qt 5 modules.
target_link_libraries (Point_Based_Renderer Qt5::Core)
target_link_libraries (Point_Based_Renderer Qt5::Gui)
target_link_libraries (Point_Based_Renderer Qt5::Widgets)
target_link_libraries (Point_Based_Renderer ${OPENGL_LIBRARIES})