// This file is part of Surface Splatting.
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
#include <vector>
#include "splat_renderer/surfel.hpp"
#include "splat_renderer/surfel_cloud.h"
#include "Array.h"
#include "utils/frustum.hpp"
#include "OglUtils.h"

// #define DEBUG
#define TUCANO_RENDERER

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
    SplatRenderer( Tucano::Camera* camera );
    virtual ~SplatRenderer();

	void begin_frame();
	void render_cloud( SurfelCloud& cloud );
    
	#ifdef TUCANO_RENDERER
		void render_cloud_tucano( const model::Array< Surfel >& surfels ) const;
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
	
	const Tucano::Camera& camera() const;

private:
    void setup_program_objects();
    void setup_filter_kernel();
    void setup_screen_size_quad();
    void setup_vertex_array_buffer_object();

	Vector2i projToWindowCoords( const Vector4f& point, const Matrix4f& viewProj, const Vector2i& viewportSize ) const;
	
    void setup_uniforms( glProgram& program, const Matrix4f& modelView );

    void render_pass( bool depth_only = false );

private:
	using RenderingVector = vector< SurfelCloud*, TbbAllocator< SurfelCloud* > >;
	
    Tucano::Camera* m_camera;
	Tucano::Frustum m_frustum;

	RenderingVector m_toRender;
	
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
};

inline void SplatRenderer::render_cloud( SurfelCloud& cloud )
{
	#ifdef DEBUG
	{
		cout << "Pushing to render list: " << endl << cloud << endl << endl;
	}
	#endif
	
	m_renderedSplats += cloud.numPoints();
	m_toRender.push_back( &cloud );
}

#ifdef TUCANO_RENDERER
	inline void SplatRenderer::render_cloud_tucano( const model::Array< Surfel >& surfels ) const
	{
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
	Vector4f min( rawMin.x(), rawMin.y(), rawMin.z(), 1 );
	Vector4f max( rawMax.x(), rawMax.y(), rawMax.z(), 1 );
	
	Vector2i viewportSize = m_camera->getViewportSize();
	
	const Matrix4f& viewProj = m_frustum.viewProj();
	
	Vector2i proj0 = projToWindowCoords( min, viewProj, viewportSize );
	Vector2i proj1 = projToWindowCoords( max, viewProj, viewportSize );
	
	Vector2i diagonal0 = proj1 - proj0;
	
	Vector3f boxSize = rawMax - rawMin;
	
	proj0 = projToWindowCoords( Vector4f( min.x() + boxSize.x(), min.y() + boxSize.y(), min.z(), 1 ), viewProj,
								viewportSize );
	proj1 = projToWindowCoords( Vector4f( max.x(), max.y(), max.z() + boxSize.z(), 1 ), viewProj, viewportSize );
	
	Vector2i diagonal1 = proj1 - proj0;
	
	float maxDiagLength = std::max( diagonal0.squaredNorm(), diagonal1.squaredNorm() );
	
	return maxDiagLength < projThresh;
}

inline void SplatRenderer::begin_frame()
{
	m_frustum.update( *m_camera );
	
	#ifndef TUCANO_RENDERER
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
	
    setup_uniforms( program, m_camera->getViewMatrix().matrix()/* * cloud.model()*/ );
	
    if (!depth_only && m_soft_zbuffer && m_ewa_filter)
    {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_1D, m_filter_kernel);

        program.set_uniform_1i("filter_kernel", 1);
    }
    
    for( SurfelCloud* cloud : m_toRender )
	{
		cloud->render();
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
			
			if (m_soft_zbuffer)
			{
				render_pass( true);
			}

			#ifndef NDEBUG
				util::OglUtils::checkOglErrors();
			#endif
			
			render_pass( false );
			
			#ifndef NDEBUG
				util::OglUtils::checkOglErrors();
			#endif

			m_toRender.clear();
			
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
		}
	#endif
	
	#ifndef NDEBUG
		util::OglUtils::checkOglErrors();
	#endif
}

inline ulong SplatRenderer::end_frame()
{
	ulong renderedSplats = m_renderedSplats;
	m_renderedSplats = 0ul;
	
	return renderedSplats;
}

inline Vector2i SplatRenderer
::projToWindowCoords( const Vector4f& point, const Matrix4f& viewProj, const Vector2i& viewportSize ) const
{
	Vector4f proj = viewProj * point;
	return Vector2i( ( proj.x() / proj.w() + 1.f ) * 0.5f * viewportSize.x(),
						( proj.y() / proj.w() + 1.f ) * 0.5f * viewportSize.y() );
}

inline const Tucano::Camera& SplatRenderer::camera() const
{
	return *m_camera;
}

#undef DEBUG

#endif // SPLATRENDER_HPP
