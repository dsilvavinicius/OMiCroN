#include "renderers/TucanoRenderingState.h"

namespace model
{
	TucanoRenderingState::TucanoRenderingState( Camera* camera, Camera* lightCamera, Mesh* mesh, const string& shaderPath,
												const int& jfpbrFrameskip, const Effect& effect )
	: RenderingState(),
	m_camera( camera ),
	m_lightCamera( lightCamera ),
	m_mesh( mesh ),
	m_jfpbrFrameskip( jfpbrFrameskip ),
	m_effect( effect ),
	m_nFrames( 0 )
	{
		updateViewProjection();
		m_frustum = new Frustum( m_viewProj );
		
		m_phong = new Phong();
		m_phong->setShadersDir( shaderPath );
		m_phong->initialize();
		
		// This color should be used when there is no color vertex attribute being used.
		glVertexAttrib4f( 2, 0.7f, 0.7f, 0.7f, 1.f );
		
		Vector2i viewportSize = m_camera->getViewportSize();
		m_jfpbr = new ImgSpacePBR( viewportSize.x(), viewportSize.y() );
		m_jfpbr->setShadersDir( shaderPath );
		m_jfpbr->initialize();
		
		m_textEffect.initialize( shaderPath + "/../Inconsolata.otf" );
	}
	
	TucanoRenderingState::~TucanoRenderingState()
	{
		delete m_jfpbr;
		delete m_phong;
		delete m_frustum;
	}
	
	void TucanoRenderingState::update()
	{
		updateViewProjection();
		m_frustum->update( m_viewProj );
	}
	
	void TucanoRenderingState::setupRendering()
	{
		glClearColor(1.0, 1.0, 1.0, 0.0);
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		
		glCullFace( GL_BACK );
		glEnable( GL_CULL_FACE );
		
		// Alpha is needed for text rendering
		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		
		glEnable( GL_DEPTH_TEST );
		
		glPointSize( 2 );
		
		m_textEffect.setColor( Vector4f( 0.f, 0.f, 0.f, 1.f ) );
		
		clearAttribs();
		//clearIndices();
		update();
	}
	
	inline unsigned int TucanoRenderingState::render()
	{
		// TODO: This code is really inefficient with all these copies among vectors. Need to fix that.
		// TODO: Indices are not needed unless the Octree itself uses indices.
		++m_nFrames;
		
		unsigned long nPoints = RenderingState::m_positions.size();
		vector< Vector4f > vertData( nPoints );
		vector< GLuint > indices( nPoints );
		
		for( unsigned long int i = 0; i < nPoints; ++i )
		{
			Vec3 pos = RenderingState::m_positions[ i ];
			vertData[ i ] = Vector4f( pos.x(), pos.y(), pos.z(), 1.f );
			indices[ i ] = i;
		}
		m_mesh->loadVertices( vertData );
		m_mesh->loadIndices( indices );
		m_mesh->loadNormals( RenderingState::m_normals );
		
		if( m_colors.size() > 0  )
		{
			for( int i = 0; i < nPoints; ++i )
			{
				Vec3 color = RenderingState::m_colors[ i ];
				vertData[ i ] = Vector4f( color( 0 ), color( 1 ), color( 2 ), 1.f );
			}
			m_mesh->loadColors( vertData );
		}

		switch( m_effect )
		{
			case PHONG: m_phong->render( *m_mesh, *m_camera, *m_lightCamera ); break;
			case JUMP_FLOODING:
			{
				bool newFrame = m_nFrames % m_jfpbrFrameskip == 0;
				m_jfpbr->render( m_mesh, m_camera, m_lightCamera, newFrame );
				
				break;
			}
		}
		
		return RenderingState::m_positions.size();
	}
	
	inline bool TucanoRenderingState::isCullable( const AlignedBox3f& box ) const
	{
		return m_frustum->isCullable( box );
	}
	
	inline bool TucanoRenderingState::isRenderable( const AlignedBox3f& box, const Float projThresh ) const
	{
// 		return false;
		const Vec3& rawMin = box.min();
		const Vec3& rawMax = box.max();
		Vector4f min( rawMin.x(), rawMin.y(), rawMin.z(), 1 );
		Vector4f max( rawMax.x(), rawMax.y(), rawMax.z(), 1 );
		
		Vector2i viewportSize = m_camera->getViewportSize();
		
		Vector2i proj0 = projToWindowCoords( min, m_viewProj, viewportSize );
		Vector2i proj1 = projToWindowCoords( max, m_viewProj, viewportSize );
		
		Vector2i diagonal0 = proj1 - proj0;
		
		Vec3 boxSize = rawMax - rawMin;
		
		proj0 = projToWindowCoords( Vector4f( min.x() + boxSize.x(), min.y() + boxSize.y(), min.z(), 1 ), m_viewProj,
									viewportSize );
		proj1 = projToWindowCoords( Vector4f( max.x(), max.y(), max.z() + boxSize.z(), 1 ), m_viewProj, viewportSize );
		
		Vector2i diagonal1 = proj1 - proj0;
		
		Float maxDiagLength = std::max( diagonal0.squaredNorm(), diagonal1.squaredNorm() );
		
		return maxDiagLength < projThresh;
	}
	
	inline void TucanoRenderingState::renderText( const Vec3& pos, const string& str )
	{
		m_textEffect.render( str, Vector4f( pos.x(), pos.y(), pos.z(), 1.f ), *m_camera );
	}
	
	inline void TucanoRenderingState::updateViewProjection()
	{
		Matrix4f view = m_camera->getViewMatrix().matrix();
		Matrix4f proj = m_camera->getProjectionMatrix();
		
		m_viewProj = proj * view;
	}
	
	inline Vector2i TucanoRenderingState
	::projToWindowCoords( const Vector4f& point, const Matrix4f& viewProj, const Vector2i& viewportSize ) const
	{
		Vector4f proj = viewProj * point;
		return Vector2i( ( proj.x() / proj.w() + 1.f ) * 0.5f * viewportSize.x(),
						 ( proj.y() / proj.w() + 1.f ) * 0.5f * viewportSize.y() );
	}
}