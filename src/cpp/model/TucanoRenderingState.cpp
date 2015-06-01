#include "TucanoRenderingState.h"

namespace model
{
	TucanoRenderingState::TucanoRenderingState( Camera* camera, Camera* lightCamera, Mesh* mesh, const Attributes& attribs,
												const string& shaderPath, const int& jfpbrFrameskip, const Effect& effect )
	: RenderingState( attribs ),
	m_camera( camera ),
	m_lightCamera( lightCamera ),
	m_mesh( mesh ),
	m_viewProj( getViewProjection() ),
	m_jfpbrFrameskip( jfpbrFrameskip ),
	m_effect( effect ),
	m_nFrames( 0 )
	{
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
		
		glCullFace( GL_BACK );
		glEnable( GL_CULL_FACE );
	}
	
	TucanoRenderingState::~TucanoRenderingState()
	{
		delete m_jfpbr;
		delete m_phong;
		delete m_frustum;
	}
	
	void TucanoRenderingState::updateFrustum()
	{
		m_viewProj = getViewProjection();
		
		m_frustum->update( m_viewProj );
		
		//cout << "New frustum: " << endl << *m_frustum << endl << endl;
	}
	
	inline unsigned int TucanoRenderingState::render()
	{
		// TODO: This code is inefficient with all these copies among vectors. Need to fix that.
		// Also, it could be better to use indices for rendering. That would need some redesign of the octree classes.
		++m_nFrames;
		
		unsigned long nPoints = RenderingState::m_positions.size();
		vector< Vector4f > vertData( nPoints );
		vector< GLuint > indices( nPoints );
		
		for( unsigned long int i = 0; i < nPoints; ++i )
		{
			Vec3 pos = RenderingState::m_positions[ i ];
			vertData[ i ] = Vector4f( pos.x, pos.y, pos.z, 1.f );
			indices[ i ] = i;
		}
		m_mesh->loadVertices( vertData );
		m_mesh->loadIndices( indices );
		
		if( RenderingState::m_attribs & Attributes::COLORS  )
		{
			for( int i = 0; i < nPoints; ++i )
			{
				Vec3 color = RenderingState::m_colors[ i ];
				vertData[ i ] = Vector4f( color.x, color.y, color.z, 1.f );
			}
			m_mesh->loadColors( vertData );
		}
		
		if( RenderingState::m_attribs & Attributes::NORMALS )
		{
			vector< Vector3f > normalData( nPoints );
			for( int i = 0; i < nPoints; ++i )
			{
				Vec3 normal = RenderingState::m_normals[ i ];
				normalData[ i ] = Vector3f( normal.x, normal.y, normal.z );
			}
			m_mesh->loadNormals( normalData );
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
	
	inline bool TucanoRenderingState::isCullable( const pair< Vec3, Vec3 >& rawBox ) const
	{
		Vec3 min = rawBox.first;
		Vec3 max = rawBox.second;
		
		Box box( Vector3f( min.x, min.y, min.z ), Vector3f( max.x, max.y, max.z ) );
		return m_frustum->isCullable( box );
	}
	
	inline bool TucanoRenderingState::isRenderable( const pair< Vec3, Vec3 >& box, const Float& projThresh )
	const
	{
		Vec3 rawMin = box.first;
		Vec3 rawMax = box.second;
		Vector4f min( rawMin.x, rawMin.y, rawMin.z, 1 );
		Vector4f max( rawMax.x, rawMax.y, rawMax.z, 1 );
		
		Vector2i viewportSize = m_camera->getViewportSize();
		
		Vector2f proj0 = projToWindowCoords( min, m_viewProj, viewportSize );
		Vector2f proj1 = projToWindowCoords( max, m_viewProj, viewportSize );
		
		Vector2f diagonal0 = proj1 - proj0;
		
		Vec3 boxSize = rawMax - rawMin;
		
		proj0 = projToWindowCoords( Vector4f( min.x() + boxSize.x, min.y() + boxSize.y, min.z(), 1 ), m_viewProj,
									viewportSize );
		proj1 = projToWindowCoords( Vector4f( max.x(), max.y(), max.z() + boxSize.z, 1 ), m_viewProj, viewportSize );
		
		Vector2f diagonal1 = proj1 - proj0;
		
		Float maxDiagLength = glm::max( diagonal0.squaredNorm(), diagonal1.squaredNorm() );
		
		return maxDiagLength < projThresh;
	}
	
	inline Matrix4f TucanoRenderingState::getViewProjection() const
	{
		Matrix4f view = m_camera->getViewMatrix().matrix();
		Matrix4f proj = m_camera->getProjectionMatrix();
		
		//cout << "Projection to renderer: " << endl << proj << endl << endl;
		
		return proj * view;
	}
	
	inline Vector2f TucanoRenderingState::projToWindowCoords( const Vector4f& point,
																			 const Matrix4f& viewProj,
																		  const Vector2i& viewportSize ) const
	{
		Vector4f proj = viewProj * point;
		Vector2f normalizedProj( proj.x() / proj.w(), proj.y() / proj.w() );
		//Vector2f windowProj = ( normalizedProj + Vector2f( 1.f, 1.f ) ) * 0.5f;
		//return Vector2f( windowProj.x() * viewportSize.x(), windowProj.y() * viewportSize.y() );
		return normalizedProj;
	}
}