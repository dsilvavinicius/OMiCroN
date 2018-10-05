##############################
# Root build description.
# Path: src/
##############################

cmake_minimum_required( VERSION 3.5 )
project( OMiCroN VERSION 0.0.1 )

# Adding package finders directory.
list( INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake )

# Find includes in corresponding build directories.
set( CMAKE_INCLUDE_CURRENT_DIR ON )

# Instruct CMake to run moc automatically when needed.
set( CMAKE_AUTOMOC ON )

#OpenMP thread number environment variable.
set( ENV{OMP_NUM_THREADS} 8 )

# Necessary flags for C++11, linkage to Qt5, OpenMP and Eigen setup.
set (CMAKE_CXX_FLAGS "-std=c++17 -fPIC -fopenmp -D_GLIBCXX_PARALLEL -DEIGEN_DONT_VECTORIZE -DEIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT")

#======== Find libs ============

find_package( OpenGL REQUIRED )
find_package( GLEW REQUIRED )
find_package( Eigen3 REQUIRED NO_MODULE )
find_package( Freetype REQUIRED )
find_package( Jsoncpp REQUIRED )
find_package( STXXL REQUIRED )

find_package( Qt5Core REQUIRED )
find_package( Qt5Gui REQUIRED )
find_package( Qt5Widgets REQUIRED )
find_package( Qt5OpenGL REQUIRED )
find_package( Qt5Sql REQUIRED )

# Compiles libraries
add_subdirectory(lib)

#============= Shader compilation ===================
# TODO: maybe this solution using GLOB is just too ugly to keep existing...

include( shader_wrap_cpp )

# GLviz shader.
file( GLOB OPENGL_SHADERS omicron/renderer/splat_renderer/shader/* )

shader_wrap_cpp( SOURCES_OPENGL_SHADERS "${OPENGL_SHADERS}" )

add_library( shader
    ${SOURCES_OPENGL_SHADERS}
    ${OPENGL_SHADERS}
)

add_dependencies( shader
    embed_resource
)

#====================================================

QT5_WRAP_UI( UI_HDRS
    omicron/ui/main_window.ui
)

# Set TBB target
add_library( Tbb::Tbb UNKNOWN IMPORTED )

set_target_properties( Tbb::Tbb
	PROPERTIES
		INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include/tbb
		IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/lib/tbb/libtbbmalloc.so.2
)

# Creates the renderer library.
add_library( Point_Based_Renderer_Lib
	omicron/memory/global_new_delete.cpp
	omicron/basic/point.cpp

	omicron/ui/gl_hidden_widget.h          omicron/ui/gl_hidden_widget.cpp
	omicron/ui/point_renderer_widget.h     omicron/ui/point_renderer_widget.cpp
	omicron/ui/main_window.h               omicron/ui/main_window.cpp
	omicron/ui/main_window.ui
	
	${UI_HDRS}
	
#	omicron/hierarchy/node_loader_thread.h
	
	omicron/basic/stream.cpp
	omicron/basic/morton_code.cpp
# 	omicron/memory/memory_info.cpp
	omicron/renderer/vsgl_info_lib.cpp
	omicron/memory/tbb_allocator.cpp
# 	omicron/Scan.cpp
# 	omicron/basic/Camera.cpp
	omicron/hierarchy/hierarchy_creation_log.cpp
	omicron/hierarchy/external_octree_data.cpp
	
	omicron/renderer/splat_renderer/buffer.cpp
	omicron/renderer/splat_renderer/framebuffer.cpp
	omicron/renderer/splat_renderer/program.cpp
	omicron/renderer/splat_renderer/program_attribute.cpp
	omicron/renderer/splat_renderer/program_finalization.cpp
	omicron/renderer/splat_renderer/shader.cpp
	omicron/renderer/splat_renderer/splat_renderer.cpp
	
	omicron/renderer/rendering_state.cpp
	omicron/renderer/tucano_rendering_state.cpp
	
	omicron/hierarchy/gpu_alloc_statistics.cpp
	
	# Tucano related code ( ugly hacks to use qmake in these files ).
	include/tucano/tucano/utils/qtflycamerawidget.hpp
	include/tucano/tucano/utils/qtplainwidget.hpp 
    
    omicron/renderer/splat_renderer/surfel_cloud.cpp
)
    
# TODO: fix include's scope. Must use PUBLIC only for interface items.
target_include_directories(Point_Based_Renderer_Lib
	PUBLIC
		# DAMN YOU TUCANO!
		${CMAKE_CURRENT_SOURCE_DIR}/include
		${CMAKE_CURRENT_SOURCE_DIR}/include/tucano
		${CMAKE_CURRENT_SOURCE_DIR}/include/tucano/tucano
		${CMAKE_CURRENT_SOURCE_DIR}/include/tucano/tucano/effects
		# END OF DAMN YOU TUCANO
		
		${EIGEN3_INCLUDE_DIRS}
		${FREETYPE_INCLUDE_DIRS}
		${STXXL_INCLUDE_DIR}
)
    
# Link all necessary libs.
target_link_libraries (Point_Based_Renderer_Lib
	Qt5::Core
	Qt5::Gui
	Qt5::Widgets
	Qt5::OpenGL
	Qt5::Sql
	${OPENGL_LIBRARIES}
	${GLEW_LIBRARIES}
	${FREETYPE_LIBRARIES}
	Tbb::Tbb
	${JSONCPP_LIBRARIES}
	${STXXL_LIBRARY}
	RPly
	shader)

# Creates renderer program and links with the renderer library.
add_executable( Point_Based_Renderer
	omicron/main.cpp
)

target_include_directories( Point_Based_Renderer
	PUBLIC
		Point_Based_Renderer_Lib
)

target_link_libraries( Point_Based_Renderer Point_Based_Renderer_Lib )

# Shader files copy target.
add_custom_target( Copy )

# Project shaders.
add_custom_command( TARGET Copy PRE_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:Point_Based_Renderer_Lib>/shaders )

# Tucano shaders.
add_custom_command( TARGET Copy PRE_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/include/tucano/tucano/effects/shaders
	$<TARGET_FILE_DIR:Point_Based_Renderer_Lib>/shaders/tucano )

# Example point cloud.
add_custom_command( TARGET Copy PRE_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/example/staypuff.ply
	$<TARGET_FILE_DIR:Point_Based_Renderer_Lib>/data/example/staypuff.ply )
	
add_dependencies( Point_Based_Renderer Copy )

# Tests directory.
add_subdirectory( test )
