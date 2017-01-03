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

cmake_policy (SET CMP0026 OLD)

# Adding package finders directory.
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Find includes in corresponding build directories.
set (CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc automatically when needed.
set (CMAKE_AUTOMOC ON)

#OpenMP thread number environment variable.
set (ENV{OMP_NUM_THREADS} 8)

# Necessary flags for C++11, linkage to Qt5, OpenMP and Eigen setup.
set (CMAKE_CXX_FLAGS "-std=c++11 -fPIC -fopenmp -D_GLIBCXX_PARALLEL -DEIGEN_DONT_VECTORIZE -DEIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT")

set (HEADER_DIR "header")
set (CPP_DIR "cpp")

set (UI_DIR "ui")
set (MODEL_DIR "model")
set (UTIL_DIR "util")

create_source_groups (UI_DIR MODEL_DIR UTIL_DIR)

setup_package(OpenGL)
setup_package(GLEW)
setup_package(Eigen3)
setup_package(SQLite3)
setup_package(Freetype)
setup_package(Jsoncpp)

# Set Qt dirs.
find_package (Qt5Core REQUIRED)
find_package (Qt5Gui REQUIRED)
find_package (Qt5Widgets REQUIRED)
find_package (Qt5OpenGL REQUIRED)
find_package (Qt5Sql REQUIRED)

set (RPLY_DIR "lib/rply")
set (JUMP_FLOODING_DIR "lib/jf-pbr")
set (TLSF_DIR "lib/tlsf")

include_directories ( ${Qt5Core_INCLUDE_DIRS}
	${Qt5Gui_INCLUDE_DIRS}
	${Qt5Widgets_INCLUDE_DIRS}
	${Qt5OpenGL_INCLUDE_DIRS}
	${Qt5Sql_INCLUDE_DIRS}
	${EIGEN3_INCLUDE_DIR}
	include/
	include/tucano/src/
	include/tucano/effects/
	${RPLY_DIR}
	${JUMP_FLOODING_DIR}
	${TLSF_DIR}
	
	${HEADER_DIR}/${UI_DIR}/
	${HEADER_DIR}/${MODEL_DIR}/
	${HEADER_DIR}/${UTIL_DIR}/
	)

# Compiles libraries
add_subdirectory(lib)

include(shader_wrap_cpp)

# GLviz shader.
file(GLOB OPENGL_SHADERS ${HEADER_DIR}/${MODEL_DIR}/splat_renderer/shader/*)

shader_wrap_cpp(SOURCES_OPENGL_SHADERS "${OPENGL_SHADERS}")

source_group("Shader Files" FILES ${OPENGL_SHADERS})

add_library(shader
    ${SOURCES_OPENGL_SHADERS}
    ${OPENGL_SHADERS}
)

add_dependencies(shader
    embed_resource
)

QT5_WRAP_UI( UI_HDRS
    ${HEADER_DIR}/${UI_DIR}/mainwindow.ui
)

# Creates the renderer library.
add_library (Point_Based_Renderer_Lib
	${HEADER_DIR}/${UI_DIR}/GLHiddenWidget.h		${CPP_DIR}/${UI_DIR}/GLHiddenWidget.cpp
	${HEADER_DIR}/${UI_DIR}/PointRendererWidget.h	${CPP_DIR}/${UI_DIR}/PointRendererWidget.cpp
	${HEADER_DIR}/${UI_DIR}/MainWindow.h			${CPP_DIR}/${UI_DIR}/MainWindow.cpp
	${HEADER_DIR}/${UI_DIR}/mainwindow.ui
	
	${UI_HDRS}
	
	${HEADER_DIR}/${UTIL_DIR}/Stream.h								${CPP_DIR}/${UTIL_DIR}/Stream.cpp
	${HEADER_DIR}/${UTIL_DIR}/PlyPointReader.h
	${HEADER_DIR}/${UTIL_DIR}/PlyPointWritter.h
	${HEADER_DIR}/${MODEL_DIR}/MortonCode.h							${CPP_DIR}/${MODEL_DIR}/MortonCode.cpp
	${HEADER_DIR}/${MODEL_DIR}/Point.h								${CPP_DIR}/${MODEL_DIR}/Point.cpp
	${HEADER_DIR}/${UTIL_DIR}/MemoryInfo.h							${CPP_DIR}/${UTIL_DIR}/MemoryInfo.cpp
	${HEADER_DIR}/${UTIL_DIR}/vsGLInfoLib.h							${CPP_DIR}/${UTIL_DIR}/vsGLInfoLib.cpp
	${HEADER_DIR}/${MODEL_DIR}/TbbAllocator.h						${CPP_DIR}/${MODEL_DIR}/TbbAllocator.cpp
	${HEADER_DIR}/${MODEL_DIR}/Scan.h								${CPP_DIR}/${MODEL_DIR}/Scan.cpp
	${HEADER_DIR}/${MODEL_DIR}/Camera.h								${CPP_DIR}/${MODEL_DIR}/Camera.cpp
	${HEADER_DIR}/${MODEL_DIR}/HierarchyCreationLog.h				${CPP_DIR}/${MODEL_DIR}/HierarchyCreationLog.cpp
	
	${HEADER_DIR}/${MODEL_DIR}/splat_renderer/buffer.cpp
	${HEADER_DIR}/${MODEL_DIR}/splat_renderer/framebuffer.cpp
	${HEADER_DIR}/${MODEL_DIR}/splat_renderer/program.cpp
	${HEADER_DIR}/${MODEL_DIR}/splat_renderer/program_attribute.cpp
	${HEADER_DIR}/${MODEL_DIR}/splat_renderer/program_finalization.cpp
	${HEADER_DIR}/${MODEL_DIR}/splat_renderer/shader.cpp
	${HEADER_DIR}/${MODEL_DIR}/splat_renderer/splat_renderer.cpp
	
	${HEADER_DIR}/${MODEL_DIR}/renderers/RenderingState.h			${CPP_DIR}/${MODEL_DIR}/renderers/RenderingState.cpp
	${HEADER_DIR}/${MODEL_DIR}/renderers/TucanoRenderingState.h		${CPP_DIR}/${MODEL_DIR}/renderers/TucanoRenderingState.cpp
	
	${HEADER_DIR}/${MODEL_DIR}/NodeLoaderThread.h
	${HEADER_DIR}/${MODEL_DIR}/GpuAllocStatistics.h			${CPP_DIR}/${MODEL_DIR}/GpuAllocStatistics.cpp
	
	# Tucano related code ( ugly hacks to use qmake in these files ).
	include/tucano/src/utils/qtflycamerawidget.hpp
	include/tucano/src/utils/qtplainwidget.hpp 
	
	# Jump-flooding related code.
	${JUMP_FLOODING_DIR}/imgSpacePBR.hpp	${JUMP_FLOODING_DIR}/imgSpacePBR.cpp
    ${JUMP_FLOODING_DIR}/point_model.hpp	${JUMP_FLOODING_DIR}/point_model.cpp 
    ${JUMP_FLOODING_DIR}/bin.cpp
    
    ${HEADER_DIR}/${MODEL_DIR}/splat_renderer/surfel_cloud.cpp)

    
# Set TBB variables

include_directories ("include/tbb")
link_directories("lib/tbb")
    
# Link all necessary libs.
target_link_libraries (Point_Based_Renderer_Lib
	Qt5::Core
	Qt5::Gui
	Qt5::Widgets
	Qt5::OpenGL
	Qt5::Sql
	${SQLITE3_LIBRARIES}
	${OPENGL_LIBRARIES}
	${GLEW_LIBRARIES}
	${FREETYPE_LIBRARIES}
	"libtbbmalloc.so"
	${JSONCPP_LIBRARIES}
	RPly
	Tlsf
	shader)

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