// // This file is part of Surface Splatting.
//
// Copyright (C) 2010, 2015 by Sebastian Lipponer.
// 
// Surface Splatting is free software: you can redistribute it and / or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// Surface Splatting is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Surface Splatting. If not, see <http://www.gnu.org/licenses/>.

#ifndef SPLATRENDER_HPP
#define SPLATRENDER_HPP

#include "program_attribute.hpp"
#include "program_finalization.hpp"

#include "framebuffer.hpp"

#include <Eigen/Core>
#include <string>
#include <list>
#include "splat_renderer/surfel.hpp"
#include "splat_renderer/surfel_cloud.h"
#include "Array.h"
#include <O1OctreeNode.h>
#include "utils/frustum.hpp"
#include "OglUtils.h"
#include "global_malloc.h"

// Substitutes the splat rendering shaders by a Tucano phong shader. Bad perfomance, but good for debugging.
// #define TUCANO_RENDERER

// Defines for debugging output.
// #define IS_RENDERABLE_DEBUG
// #define RENDERING_DEBUG
// #define GL_ERROR_DEBUG
// #define LOCK_DEBUG

#ifdef TUCANO_RENDERER
	#include "phongshader.hpp"
#endif

class UniformBufferRaycast : public GLviz::glUniformBuffer
{

public:
    UniformBufferRaycast();

    void set_buffer_data(Eigen::Matrix4f const& projection_matrix_inv,
        GLint const* viewport);
};

class UniformBufferFrustum : public GLviz::glUniformBuffer
{

public:
    UniformBufferFrustum();

    void set_buffer_data(Eigen::Vector4f const* frustum_plane);
};

class UniformBufferParameter : public GLviz::glUniformBuffer
{

public:
    UniformBufferParameter();

    void set_buffer_data(Eigen::Vector3f const& color, float shininess,
        float radius_scale, float ewa_radius, float epsilon);
};

class SplatRenderer
{

public:
	using Node = O1OctreeNode< Surfel >;
	
    SplatRenderer( Tucano::Camera* camera, const Vector3f& modelCentroid = Vector3f::Zero() );
    virtual ~SplatRenderer();

	void begin_frame();
	
	/** Removes the node from the rendering list if it is currently being referenced by the current rendering list iterator. */
	void eraseFromList( const Node& node );
	
	/** Resets the inserting iterator to the beginning of the rendering list. */
	void resetIterator();
	
	/** Inserts into the rendering list after the current iterator to the rendering list. */
	void render( Node& node );
    
	#ifdef TUCANO_RENDERER
		void render_cloud_tucano( Node& node ) const;
	#endif
	
	void render_frame();
	ulong end_frame();
	
	bool isCullable( const AlignedBox3f& box ) const;
	bool isRenderable( const AlignedBox3f& box, const float projThresh ) const;
	
    bool smooth() const;
    void set_smooth(bool enable = true);

    bool backface_culling() const;
    void set_backface_culling(bool enable = true);

    bool soft_zbuffer() const;
    void set_soft_zbuffer(bool enable = true);

    float soft_zbuffer_epsilon() const;
    void set_soft_zbuffer_epsilon(float epsilon);

    unsigned int pointsize_method() const;
    void set_pointsize_method(unsigned int pointsize_method);

    bool ewa_filter() const;
    void set_ewa_filter(bool enable = true);

    bool multisample() const;
    void set_multisample(bool enable = true);

    float const* material_color() const;
    void set_material_color(float const* color_ptr);
    float material_shininess() const;
    void set_material_shininess(float shininess);

    float radius_scale() const;
    void set_radius_scale(float radius_scale);

    float ewa_radius() const;
    void set_ewa_radius(float ewa_radius);

    void reshape(int width, int height);
	
	void toggleFboSave();
	
	void toggleModelMatrixUse();
	
	const Tucano::Camera& camera() const;

private:
    void setup_program_objects();
    void setup_filter_kernel();
    void setup_screen_size_quad();
    void setup_vertex_array_buffer_object();

	Vector2f projToNormDeviceCoords( const Vector4f& point, const Matrix4f& viewProj ) const;
	
    void setup_uniforms( glProgram& program, const Matrix4f& modelView );

    void render_pass( bool depth_only = false );

	/**
	* @brief Saves the buffer to a PPM image
	* @param attach FBO attachment to save as image (default is 0)
	*/
	void saveFbo( int attach = 0 );
	
private:
	using RenderingList = list< Node*, TbbAllocator< Node* > >;
	using RenderingListIter = typename RenderingList::iterator;
	
    Tucano::Camera* m_camera;
	Tucano::Frustum m_frustum;

	// Spaguetti.
	Affine3f m_model;
	Vector3f m_modelCentroid;
	
	RenderingList m_toRender;
	RenderingListIter m_toRenderIter;
	
    GLuint m_rect_vertices_vbo, m_rect_texture_uv_vbo,
        m_rect_vao, m_filter_kernel;

    ProgramAttribute m_visibility, m_attribute;
    ProgramFinalization m_finalization;

    model::Framebuffer m_fbo;

    bool m_soft_zbuffer, m_backface_culling, m_smooth, m_ewa_filter, m_multisample;
    unsigned int m_pointsize_method;
    Eigen::Vector3f m_color;
    float m_epsilon, m_shininess, m_radius_scale, m_ewa_radius;

    GLviz::UniformBufferCamera m_uniform_camera;
    UniformBufferRaycast m_uniform_raycast;
    UniformBufferFrustum m_uniform_frustum;
    UniformBufferParameter m_uniform_parameter;
	
	// Stats.
	ulong m_renderedSplats;
	
	// Members related with saving FBO in disk.
	// Flag that indicates if the fbo should be saved in Disk.
	bool m_saveFboFlag;
	
	// Indicates that the model matrix should be used.
	bool m_useModelMatrix;
	
	// FBO file current suffix.
	atomic_int m_diskFileSuffix;
};

inline void SplatRenderer::eraseFromList( const Node& node )
{
	#ifdef RENDERING_DEBUG
	{
		if( m_toRenderIter != m_toRender.end() )
		{
			stringstream ss; ss << "m_toRenderIter: " << *m_toRenderIter << " cloud: "
				<< ( *m_toRenderIter )->cloud() << " parameter: isLoaded ? " << node.isLoaded() << " " << endl << endl;
			HierarchyCreationLog::logDebugMsg( ss.str() );
			HierarchyCreationLog::flush();
		}
	}
	#endif
	
	if( m_toRenderIter != m_toRender.end() && node.isLoaded() && node.cloud() == ( *m_toRenderIter )->cloud() )
	{
		#ifdef RENDERING_DEBUG
		{
			stringstream ss; ss << "Erasing: " << endl << node.cloud() << endl << endl;
			HierarchyCreationLog::logDebugMsg( ss.str() );
		}
		#endif
		
		m_toRenderIter = m_toRender.erase( m_toRenderIter );
		
		#ifdef RENDERING_DEBUG
		{
			stringstream ss; ss << "Pointing now to: ";
			if( m_toRenderIter != m_toRender.end() )
			{
				ss << ( *m_toRenderIter )->cloud() << endl;
			}
			else
			{
				ss << " end" << endl << endl;
			}
			HierarchyCreationLog::logDebugMsg( ss.str() );
		}
		#endif
	}
}

inline void SplatRenderer::resetIterator()
{
	#ifdef RENDERING_DEBUG
	{
		HierarchyCreationLog::logDebugMsg( "Resetting iterator.\n\n" );
	}
	#endif
	
	m_toRenderIter = m_toRender.begin();
}

inline void SplatRenderer::render( Node& node )
{
	m_renderedSplats += node.cloud().numPoints();
	
	if( m_toRenderIter != m_toRender.end() )
	{
		if( node.cloud() != ( *m_toRenderIter )->cloud() )
		{
			#ifdef RENDERING_DEBUG
			{
				stringstream ss; ss << "Pushing to render list (different): " << endl << node.cloud() << endl << endl;
				HierarchyCreationLog::logDebugMsg( ss.str() );
			}
			#endif
			
			m_toRender.insert( m_toRenderIter, &node );
		}
		else
		{
			m_toRenderIter++;
		}
	}
	else
	{
		#ifdef RENDERING_DEBUG
		{
			stringstream ss; ss << "Pushing to render list (list end): " << endl << node.cloud() << endl << endl;
			HierarchyCreationLog::logDebugMsg( ss.str() );
		}
		#endif
		
		m_toRender.insert( m_toRenderIter, &node );
	}
}

#ifdef TUCANO_RENDERER
	inline void SplatRenderer::render_cloud_tucano( Node& node ) const
	{
		model::Array< Surfel >& surfels = node.getContents()
		
		Mesh mesh;
		mesh.selectPrimitive( Mesh::POINT );
		vector< Vector4f > vertices;
		vector< Vector3f > normals;
		
		for( const Surfel& surfel : surfels )
		{
			vertices.push_back( Vector4f( surfel.c.x(), surfel.c.y(), surfel.c.z(), 1.f ) );
			normals.push_back( surfel.u.cross( surfel.v ) );
		}
		mesh.loadVertices( vertices );
		mesh.loadNormals( normals );
		
		Effects::Phong phong;
		phong.setShadersDir( "shaders/tucano/" );
		phong.initialize();
		
		phong.render( mesh, *m_camera, *m_camera );
	}
#endif

inline bool SplatRenderer::isCullable( const AlignedBox3f& box ) const
{
	return m_frustum.isCullable( box );
}

inline bool SplatRenderer::isRenderable( const AlignedBox3f& box, const float projThresh ) const
{
	const Vector3f& rawMin = box.min();
	const Vector3f& rawMax = box.max();
	
	float delta = 1e-6f;
	
	// No points in plane z = 0 are allowed, since their homogeneous projections are undefined.
	Vector4f min( rawMin.x(), rawMin.y(), ( fabs( rawMin.z() ) < delta ) ? rawMax.z() : rawMin.z(), 1 );
	Vector4f max( rawMax.x(), rawMax.y(), ( fabs( rawMax.z() ) < delta ) ? rawMin.z() : rawMax.z(), 1  );
	
	const Matrix4f& viewProj = m_frustum.viewProj();
	
	Vector2f proj00 = projToNormDeviceCoords( min, viewProj );
	Vector2f proj01 = projToNormDeviceCoords( max, viewProj );
	
	Vector2f diagonal0 = proj01 - proj00;
	
	Vector3f boxSize = rawMax - rawMin;
	
	Vector2f proj10 = projToNormDeviceCoords( Vector4f( min.x(), max.y(), max.z(), 1 ), viewProj );
	Vector2f proj11 = projToNormDeviceCoords( Vector4f( max.x(), min.y(), min.z(), 1 ), viewProj );
	
	Vector2f diagonal1 = proj11 - proj10;
	
	float maxDiagLength = std::max( diagonal0.squaredNorm(), diagonal1.squaredNorm() );
	
	#ifdef IS_RENDERABLE_DEBUG
	{
		stringstream ss; ss << "Viewproj: " << endl << viewProj << endl << "Min: " << endl << min << endl << "Max: "
			<< max << endl << "Proj00: " << endl << proj00 << endl << "Proj01" << endl << proj01 << endl << "Proj10"
			<< endl << proj10 << endl << "Proj11" << endl << proj11 << endl << "Diagonal0: " << diagonal0
			<< " Diagonal1 : " << diagonal1 << " Max diag norm: " << maxDiagLength << endl << endl;
		HierarchyCreationLog::logDebugMsg( ss.str() );
	}
	#endif
	
	return maxDiagLength < projThresh;
}

inline void SplatRenderer::begin_frame()
{
	m_frustum.update( *m_camera );
	
	#if !defined TUCANO_RENDERER && !defined PROGRAM_ATTRIBUTE_DEBUG
		m_fbo.bind();
	#endif
		
	glDepthMask(GL_TRUE);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClearDepth(1.0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

inline void SplatRenderer::render_pass( bool depth_only )
{ 
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);

    if (!depth_only && m_soft_zbuffer)
    {
        glEnable(GL_BLEND);
        glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE);
    }

    glProgram &program = depth_only ? m_visibility : m_attribute;

    program.use();
	
    if (depth_only)
    {
        glDepthMask(GL_TRUE);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    }
    else
    {
        if (m_soft_zbuffer)
            glDepthMask(GL_FALSE);
        else
            glDepthMask(GL_TRUE);

        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    }
	
	Matrix4f model( Matrix4f::Identity() );
	
	if( m_useModelMatrix )
	{
		m_model.rotate( AngleAxisf( 0.001 * M_PI, Vector3f::UnitZ() ) );
		model = m_model.matrix();
	}
	
	setup_uniforms( program, m_camera->getViewMatrix().matrix() * model );
	
    if (!depth_only && m_soft_zbuffer && m_ewa_filter)
    {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_1D, m_filter_kernel);

        program.set_uniform_1i("filter_kernel", 1);
    }
    
	for( Node* node : m_toRender )
	{
		node->cloud().render();
	}
	
    program.unuse();

    glDisable(GL_PROGRAM_POINT_SIZE);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
}

inline void SplatRenderer::render_frame()
{
	#ifndef TUCANO_RENDERER
		if( !m_toRender.empty() )
		{
			if (m_multisample)
			{
				glEnable(GL_MULTISAMPLE);
				glEnable(GL_SAMPLE_SHADING);
				glMinSampleShading(4.0);
			}

			#ifndef NDEBUG
				util::OglUtils::checkOglErrors();
			#endif
			
			#ifndef PROGRAM_ATTRIBUTE_DEBUG
				if (m_soft_zbuffer)
				{
					render_pass( true );
				}
			#endif

			#ifndef NDEBUG
				util::OglUtils::checkOglErrors();
			#endif
			
			render_pass( false );
			
			#ifndef NDEBUG
				util::OglUtils::checkOglErrors();
			#endif
			
			#ifndef PROGRAM_ATTRIBUTE_DEBUG
				if (m_multisample)
				{
					glDisable(GL_MULTISAMPLE);
					glDisable(GL_SAMPLE_SHADING);
				}
				
				m_fbo.unbind();

				if (m_multisample)
				{
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_fbo.color_texture());

					if (m_smooth)
					{
						glActiveTexture(GL_TEXTURE1);
						glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_fbo.normal_texture());

						glActiveTexture(GL_TEXTURE2);
						glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_fbo.depth_texture());
					}
				}
				else
				{
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, m_fbo.color_texture());

					if (m_smooth)
					{
						glActiveTexture(GL_TEXTURE1);
						glBindTexture(GL_TEXTURE_2D, m_fbo.normal_texture());

						glActiveTexture(GL_TEXTURE2);
						glBindTexture(GL_TEXTURE_2D, m_fbo.depth_texture());
					}
				}

				m_finalization.use();
				
				try
				{
					setup_uniforms(m_finalization, m_camera->getViewMatrix().matrix());
					m_finalization.set_uniform_1i("color_texture", 0);

					if (m_smooth)
					{
						m_finalization.set_uniform_1i("normal_texture", 1);
						m_finalization.set_uniform_1i("depth_texture", 2);
					}
				}
				catch (uniform_not_found_error const& e)
				{
					std::cerr << "Warning: Failed to set a uniform variable." << std::endl
						<< e.what() << std::endl;
				}

				glBindVertexArray(m_rect_vao);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
				glBindVertexArray(0);
				
				if( m_saveFboFlag )
				{
					saveFbo();
				}
			#endif
		}
	#endif
	
	#if defined GL_ERROR_DEBUG || !defined NDEBUG
		util::OglUtils::checkOglErrors();
	#endif
}

inline ulong SplatRenderer::end_frame()
{
	ulong renderedSplats = m_renderedSplats;
	m_renderedSplats = 0ul;
	
	#ifdef RENDERING_DEBUG
	{
		stringstream ss; ss << "Rendered nodes: " << distance( m_toRender.begin(), m_toRender.end() ) << endl << endl;
		HierarchyCreationLog::logDebugMsg( ss.str() );
	}
	#endif
	
	return renderedSplats;
}

inline Vector2f SplatRenderer
::projToNormDeviceCoords( const Vector4f& point, const Matrix4f& viewProj ) const
{
	Vector4f proj = viewProj * point;
	
	#ifdef IS_RENDERABLE_DEBUG
	{
		stringstream ss; ss << "P: " << endl << point << endl << "proj: " << endl << proj << endl << endl;
		HierarchyCreationLog::logDebugMsg( ss.str() );
	}
	#endif
	
	return Vector2f( proj.x() / proj.w(), proj.y() / proj.w() );
}

inline void SplatRenderer::toggleFboSave()
{
	m_saveFboFlag = !m_saveFboFlag;
}

inline void SplatRenderer::toggleModelMatrixUse()
{
	m_useModelMatrix = !m_useModelMatrix;
}

inline void SplatRenderer::saveFbo( int attach )
{
	mutex mtx;
	condition_variable condVar;
	bool isCopyDone = false;
	
	GLint dims[4] = {0};
	glGetIntegerv( GL_VIEWPORT, dims );
	uint width = dims[ 2 ];
	uint height = dims[ 3 ];

	const size_t format_nchannels = 3;
	size_t imgSize = format_nchannels * sizeof( GLubyte ) * width * height;
	GLubyte* pixels = new GLubyte[ imgSize ];
	
	glReadBuffer( GL_COLOR_ATTACHMENT0 );
	glReadPixels( 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels );
	
	thread t(
		[&]()
		{
			GLubyte* threadPixels;
			uint threadWidth, threadHeight;
			{
				unique_lock< std::mutex > lock( mtx );
				threadPixels = new GLubyte[ imgSize ];
				memcpy( threadPixels, pixels, imgSize );
				
				threadWidth = width;
				threadHeight = height;
				
				isCopyDone = true;
				lock.unlock();
				
				condVar.notify_one();
			}
			
			++m_diskFileSuffix;
			
			stringstream filename; filename << "/home/vinicius/Vídeos/OMiCroN/fbo" << m_diskFileSuffix << ".ppm";
			
			ofstream out_stream;
			out_stream.open( filename.str() );
			out_stream << "P6\n";
			out_stream << threadWidth << " " << threadHeight << "\n";
			out_stream << "255\n";

			for( int i = threadHeight - 1; i >= 0; --i )
			{
				out_stream.write( ( char* ) threadPixels + i * threadWidth * 3, threadWidth * 3 );
			}
			out_stream.close();
			
			delete[] threadPixels;
		}
	);
	t.detach();
	
	{
		unique_lock< std::mutex > lock( mtx );
		condVar.wait( lock, [ & ](){ return isCopyDone; } );	
	}
	
	delete[] pixels;
}

inline const Tucano::Camera& SplatRenderer::camera() const
{
	return *m_camera;
}

#undef IS_RENDERABLE_DEBUG
#undef RENDERING_DEBUG
#undef GL_ERROR_DEBUG

#endif // SPLATRENDER_HPP
