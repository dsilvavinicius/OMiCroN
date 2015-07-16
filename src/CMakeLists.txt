##############################
# Root build description.
# Path: src/
##############################

cmake_minimum_required (VERSION 2.8.12)
project (Point_Based_Renderer)

# Setups the given package, trying to find it and sets includes and linking directories.
macro (setup_package package)
	find_package (${package} REQUIRED)
	
	string(TOUPPER "${package}" up_package)
	
	include_directories (${${up_package}_INCLUDE_DIRS})
	link_directories(${${up_package}_LIBRARY_DIRS})
	add_definitions(${${up_package}_DEFINITIONS})
	
	if(NOT ${up_package}_FOUND)
		message(WARNING "${up_package} not found")
	endif(NOT ${up_package}_FOUND)
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
# Necessary flags for C++11, linkage to Qt5, OpenMP and Eigen setup.
SET (CMAKE_CXX_FLAGS "-std=c++11 -fPIC -fopenmp -DEIGEN_DONT_VECTORIZE -DEIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT")

set (HEADER_DIR "header")
set (CPP_DIR "cpp")

set (UI_DIR "ui")
set (MODEL_DIR "model")
set (UTIL_DIR "util")

create_source_groups (UI_DIR MODEL_DIR UTIL_DIR)

setup_package(OpenGL)
setup_package(GLM)
setup_package(GLEW)
setup_package(Eigen3)
setup_package(SQLite3)

# Set Qt dirs.
find_package (Qt5Core REQUIRED)
find_package (Qt5Gui REQUIRED)
find_package (Qt5Widgets REQUIRED)
find_package (Qt53D REQUIRED)
find_package (Qt5Sql REQUIRED)

set (RPLY_DIR "lib/rply")
set (JUMP_FLOODING_DIR "lib/jf-pbr")

include_directories ( ${Qt5Core_INCLUDE_DIRS}
	${Qt5Gui_INCLUDE_DIRS}
	${Qt5Widgets_INCLUDE_DIRS}
	${Qt53D_INCLUDE_DIRS}
	${Qt5Sql_INCLUDE_DIRS}
	${EIGEN3_INCLUDE_DIR}
	include/
	include/tucano/src/
	include/tucano/effects/
	${RPLY_DIR}
	${JUMP_FLOODING_DIR}
	
	${HEADER_DIR}/${UI_DIR}/
	${HEADER_DIR}/${MODEL_DIR}/
	${HEADER_DIR}/${UTIL_DIR}/
	)

# Compiles libraries
add_subdirectory(lib)

QT5_WRAP_UI( UI_HDRS
    ${HEADER_DIR}/${UI_DIR}/mainwindow.ui
)

# Creates the renderer library.
add_library (Point_Based_Renderer_Lib
	${HEADER_DIR}/${UI_DIR}/PointRendererWindow.h	${CPP_DIR}/${UI_DIR}/PointRendererWindow.cpp
	${HEADER_DIR}/${UI_DIR}/PointRendererWidget.h	${CPP_DIR}/${UI_DIR}/PointRendererWidget.cpp
	${HEADER_DIR}/${UI_DIR}/MainWindow.h			${CPP_DIR}/${UI_DIR}/MainWindow.cpp
	${HEADER_DIR}/${UI_DIR}/mainwindow.ui
	
	${UI_HDRS}
	
	${HEADER_DIR}/${UTIL_DIR}/Stream.h						${CPP_DIR}/${UTIL_DIR}/Stream.cpp
	${HEADER_DIR}/${UTIL_DIR}/MemoryInfo.h					${CPP_DIR}/${UTIL_DIR}/MemoryInfo.cpp
	
	${HEADER_DIR}/${MODEL_DIR}/MemoryManager.h				${CPP_DIR}/${MODEL_DIR}/MemoryManager.cpp
	${HEADER_DIR}/${MODEL_DIR}/Scan.h						${CPP_DIR}/${MODEL_DIR}/Scan.cpp
	${HEADER_DIR}/${MODEL_DIR}/Camera.h						${CPP_DIR}/${MODEL_DIR}/Camera.cpp
	${HEADER_DIR}/${MODEL_DIR}/MortonCode.h					${CPP_DIR}/${MODEL_DIR}/MortonCode.cpp
	${HEADER_DIR}/${MODEL_DIR}/Point.h						${CPP_DIR}/${MODEL_DIR}/Point.cpp
	${HEADER_DIR}/${MODEL_DIR}/ExtendedPoint.h				${CPP_DIR}/${MODEL_DIR}/ExtendedPoint.cpp
	${HEADER_DIR}/${MODEL_DIR}/RenderingState.h				${CPP_DIR}/${MODEL_DIR}/RenderingState.cpp
	${HEADER_DIR}/${MODEL_DIR}/TransientRenderingState.h	${CPP_DIR}/${MODEL_DIR}/TransientRenderingState.cpp
	${HEADER_DIR}/${MODEL_DIR}/QtRenderingState.h			${CPP_DIR}/${MODEL_DIR}/QtRenderingState.cpp
	${HEADER_DIR}/${MODEL_DIR}/TucanoRenderingState.h		${CPP_DIR}/${MODEL_DIR}/TucanoRenderingState.cpp
	
	# Tucano related code
	include/tucano/src/utils/qtflycamerawidget.hpp #ugly hack to use qmake in this file.
	
	# Jump-flooding related code.
	${JUMP_FLOODING_DIR}/imgSpacePBR.hpp	${JUMP_FLOODING_DIR}/imgSpacePBR.cpp
    ${JUMP_FLOODING_DIR}/point_model.hpp	${JUMP_FLOODING_DIR}/point_model.cpp 
    ${JUMP_FLOODING_DIR}/bin.cpp )

# Link all necessary libs.
target_link_libraries (Point_Based_Renderer_Lib
	Qt5::Core
	Qt5::Gui
	Qt5::Widgets
	Qt5::3D
	Qt5::Sql
	${SQLITE3_LIBRARIES}
	${OPENGL_LIBRARIES}
	${GLEW_LIBRARIES}
	RPly)

# Creates renderer program and links with the renderer library.
add_executable (Point_Based_Renderer
	${CPP_DIR}/main.cpp
	./CMakeLists.txt)
target_include_directories (Point_Based_Renderer PUBLIC Point_Based_Renderer_Lib)
target_link_libraries (Point_Based_Renderer Point_Based_Renderer_Lib)

# Shader files copy target.
add_custom_target( Copy )
get_target_property( Renderer_Location Point_Based_Renderer LOCATION )
get_filename_component( Renderer_Dir ${Renderer_Location} PATH)

# Project shaders.
add_custom_command( TARGET Copy PRE_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/shaders ${Renderer_Dir}/shaders )

# Tucano shaders.
add_custom_command( TARGET Copy PRE_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/include/tucano/effects/shaders
	${Renderer_Dir}/shaders/tucano )
	
# Jump-flooding shaders.
add_custom_command( TARGET Copy PRE_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/lib/jf-pbr/shaders
	${Renderer_Dir}/shaders/tucano )

# Example point cloud.
add_custom_command( TARGET Copy PRE_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/example/staypuff.ply
	${Renderer_Dir}/data/example/staypuff.ply )
	
add_dependencies( Point_Based_Renderer Copy )

# Tests directory.
add_subdirectory (test)