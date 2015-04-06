#ifndef TUCANO_RENDERING_STATE_H
#define TUCANO_RENDERING_STATE_H

#include <tucano.hpp>
#include <phongshader.hpp>
#include <imgSpacePBR.hpp>
#include "RenderingState.h"
#include "Frustum.h"

using namespace Tucano;
using namespace Effects;

namespace model
{
	/** RenderingState using Tucano library ( @link http://terra.lcg.ufrj.br/tucano/ ). */
	template< typename Vec3, typename Float >
	class TucanoRenderingState
	: public RenderingState< Vec3, Float >
	{
		using RenderingState = model::RenderingState< Vec3, Float >;
		using Box = AlignedBox< Float, 3 >;
	public:
		enum Effect
		{
			PHONG,
			JUMP_FLOODING
		};
		
		TucanoRenderingState( Trackball* camTrackball, Trackball* lightTrackball , Mesh* mesh, const Attributes& attribs,
							  const string& shaderPath, const int& jfpbrFrameskip = 1, const Effect& effect = PHONG );
		
		~TucanoRenderingState();
		
		/** Updates the frustum after changes on camera. */
		void updateFrustum();
		
		virtual unsigned int render();
		
		virtual bool isCullable( const pair< Vec3, Vec3 >& box ) const;
		
		/** This implementation will compare the size of the maximum box diagonal in window coordinates with the projection
		 *	threshold.
		 *	@param projThresh is the threshold of the squared size of the maximum box diagonal in window coordinates. */
		virtual bool isRenderable( const pair< Vec3, Vec3 >& box, const Float& projThresh ) const;
	
		/** Gets the image space pbr effect. The caller is reponsable for the correct usage.*/
		ImgSpacePBR& getJumpFlooding() { return *m_jfpbr; }
		
		/** Gets the phong effect. The caller is reponsable for the correct usage.*/
		Phong& getPhong() { return *m_phong; }
		
		/** Changes the effect used to render the points. */
		void selectEffect( const Effect& effect ) { m_effect = effect; }
		
		void setJfpbrFrameskip( const int& value ) { m_jfpbrFrameskip = value; }
		
	protected:
		/** Acquires current traball's view-projection matrix. */
		Matrix4f getViewProjection() const;
		
		/** Projects the point in world coordinates to window coordinates. */
		Vector2f projToWindowCoords( const Vector4f& point, const Matrix4f& viewProjection, const Vector2i& viewportSize )
		const;
		
		Frustum* m_frustum;
		Trackball* m_camTrackball;
		Trackball* m_lightTrackball;
		
		Matrix4f m_viewProj;
		
		Mesh* m_mesh;
		Phong* m_phong;
		ImgSpacePBR *m_jfpbr;
		
		Effect m_effect;
		
		/** Frameskip for the Jump Flooding effect. */
		int m_jfpbrFrameskip;
		
		/** Frame counter. Used in order to skip frames properly. */
		unsigned int m_nFrames;
	};
	
	template< typename Vec3, typename Float >
	TucanoRenderingState< Vec3, Float >::TucanoRenderingState( Trackball*  camTrackball, Trackball* lightTrackball,
															   Mesh* mesh, const Attributes& attribs,
															const string& shaderPath, const int& jfpbrFrameskip, 
															const Effect& effect )
	: RenderingState( attribs ),
	m_camTrackball( camTrackball ),
	m_lightTrackball( lightTrackball ),
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
		
		Vector2i viewportSize = m_camTrackball->getViewportSize();
		m_jfpbr = new ImgSpacePBR( viewportSize.x(), viewportSize.y() );
		m_jfpbr->setShadersDir( shaderPath );
		m_jfpbr->initialize();
	}
	
	template< typename Vec3, typename Float >
	TucanoRenderingState< Vec3, Float >::~TucanoRenderingState()
	{
		delete m_jfpbr;
		delete m_phong;
		delete m_frustum;
	}
	
	template< typename Vec3, typename Float >
	void TucanoRenderingState< Vec3, Float >::updateFrustum()
	{
		m_viewProj = getViewProjection();
		
		m_frustum->update( m_viewProj );
		
		//cout << "New frustum: " << endl << *m_frustum << endl << endl;
	}
	
	template< typename Vec3, typename Float >
	inline unsigned int TucanoRenderingState< Vec3, Float >::render()
	{
		// TODO: This code is inefficient with all these copies among vectors. Need to fix that.
		// Also, it could be better to use indices for rendering. That would need some redesign of the octree classes.
		++m_nFrames;
		
		int nPoints = RenderingState::m_positions.size();
		vector< Vector4f > vertData( nPoints );
		vector< GLuint > indices( nPoints );
		
		for( int i = 0; i < nPoints; ++i )
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
			case PHONG: m_phong->render( *m_mesh, *m_camTrackball, *m_lightTrackball ); break;
			case JUMP_FLOODING:
			{
				bool newFrame = m_nFrames % m_jfpbrFrameskip == 0;
				m_jfpbr->render( m_mesh, m_camTrackball, m_lightTrackball, newFrame );
				
				break;
			}
		}
		
		return RenderingState::m_positions.size();
	}
	
	template< typename Vec3, typename Float >
	inline bool TucanoRenderingState< Vec3, Float >::isCullable( const pair< Vec3, Vec3 >& rawBox ) const
	{
		Vec3 min = rawBox.first;
		Vec3 max = rawBox.second;
		
		Box box( Vector3f( min.x, min.y, min.z ), Vector3f( max.x, max.y, max.z ) );
		return m_frustum->isCullable( box );
	}
	
	template< typename Vec3, typename Float >
	inline bool TucanoRenderingState< Vec3, Float >::isRenderable( const pair< Vec3, Vec3 >& box, const Float& projThresh )
	const
	{
		Vec3 rawMin = box.first;
		Vec3 rawMax = box.second;
		Vector4f min( rawMin.x, rawMin.y, rawMin.z, 1 );
		Vector4f max( rawMax.x, rawMax.y, rawMax.z, 1 );
		
		Vector2i viewportSize = m_camTrackball->getViewportSize();
		
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
	
	template< typename Vec3, typename Float >
	inline Matrix4f TucanoRenderingState< Vec3, Float >::getViewProjection() const
	{
		Matrix4f view = m_camTrackball->getViewMatrix().matrix();
		Matrix4f proj = m_camTrackball->getProjectionMatrix();
		
		//cout << "Projection to renderer: " << endl << proj << endl << endl;
		
		return proj * view;
	}
	
	template< typename Vec3, typename Float >
	inline Vector2f TucanoRenderingState< Vec3, Float >::projToWindowCoords( const Vector4f& point,
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

#endif