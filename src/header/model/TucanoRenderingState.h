#ifndef TUCANO_RENDERING_STATE_H
#define TUCANO_RENDERING_STATE_H

#include <tucano/src/tucano.hpp>
#include <tucano/effects/phongshader.hpp>
#include "RenderingState.h"
#include "Frustum.h"

using namespace Tucano;

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
		/** @param trackball is the trackball, which has the view-projection matrix.
		 *	@param attribs is the vertex attributes setup flag. */
		TucanoRenderingState( Trackball const *  camTrackball, Trackball const * lightTrackball , const Attributes& attribs );
		
		~TucanoRenderingState();
		
		/** Updates the frustum after changes on camera. */
		void updateFrustum();
		
		virtual unsigned int render();
		
		virtual bool isCullable( const pair< Vec3, Vec3 >& box ) const;
		
		virtual bool isRenderable( const pair< Vec3, Vec3 >& box, const Float& projThresh ) const;
	
	private:
		/** Acquires current traball's view-projection matrix. */
		Matrix4f getViewProjection() const;
		
		Frutum* m_frustum;
		Trackball const * m_camTrackball;
		Trackball const * m_lightTrackball;
		
		Mesh* m_mesh;
		Phong* m_phong;
	};
	
	template< typename Vec3, typename Float >
	TucanoRenderingState< Vec3, Float >::TucanoRenderingState( Trackball const *  camTrackball, Trackball const * lightTrackball,
															   const Attributes& attribs, const string& shaderPath )
	: RenderingState( attribs ),
	m_camTrackball( camTrackball ),
	m_lightTrackball( lightTrackball )
	{
		Matrix4f viewProj = getViewProjection();
		m_frustum = new Frustum( viewProj );
		m_mesh = new Mesh();
		
		m_phong = new Phong();
		m_phong->setShadersDir( shaderPath );
		m_phong->initialize();
	}
	
	template< typename Vec3, typename Float >
	TucanoRenderingState< Vec3, Float >::~TucanoRenderingState()
	{
		delete m_phong;
		delete m_mesh;
		delete m_frustum;
	}
	
	template< typename Vec3, typename Float >
	void TucanoRenderingState< Vec3, Float >::updateFrustum()
	{
		Matrix4f viewProj = getViewProjection();
		
		m_frustum->update( viewProj );
	}
	
	template< typename Vec3, typename Float >
	inline unsigned int TucanoRenderingState< Vec3, Float >::render()
	{
		// TODO: This code is inefficient with all these copies among vectors. Need to fix that.
		// Also, it could be better to use indices for rendering. That would need some redesign of the octree classes.
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
		
		m_phong->render( m_mesh, m_camTrackball, m_lightTrackball );
	}
	
	template< typename Vec3, typename Float >
	inline bool TucanoRenderingState< Vec3, Float >::isCullable( const pair< Vec3, Vec3 >& rawBox )
	{
		Vec3 min = rawBox.first;
		Vec3 max = rawBox.second;
		
		Box box( Vector3f( min.x, min.y, min.z ), Vector3f( max.x, max.y, max.z ) );
		return m_frustum->isCullable( box );
	}
	
	template< typename Vec3, typename Float >
	inline bool TucanoRenderingState< Vec3, Float >::isRenderable( const pair< Vec3, Vec3 >& box, const Float& projThresh )
	{
		Vec3 rawMin = box.first;
		Vec3 rawMax = box.second;
		Vector4f min( rawMin.x, rawMin.y, rawMin.z, 1 );
		Vector4f max( rawMax.x, rawMax.y, rawMax.z, 1 );
		
		Matrix4f viewProj = getViewProjection();
		
		Vector4f proj0 = viewProj * min;
		Vector2f normalizedProj0( proj0 / proj0.w() );
		
		Vector4f proj1 = viewProj * max;
		Vector2f normalizedProj1( proj1 / proj1.w() );
		
		Vector2f diagonal0 = normalizedProj1 - normalizedProj0;
		
		Vec3 boxSize = rawMax - rawMin;
		
		proj0 = viewProj * Vector4f( min.x() + boxSize.x(), min.y() + boxSize.y(), min.z(), 1 );
		normalizedProj0 = Vector2f( proj0 / proj0.w() );
		
		proj1 = viewProj * Vector4f( max.x(), max.y(), max.z() + boxSize.z(), 1 );
		normalizedProj1 = Vector2f( proj1 / proj1.w() );
		
		Vector2f diagonal1 = normalizedProj1 - normalizedProj0;
		
		Float maxDiagLength = glm::max( diagonal0.squaredNorm(), diagonal1.squaredNorm() );
		
		return maxDiagLength < projThresh;
	}
	
	template< typename Vec3, typename Float >
	inline Matrix4f TucanoRenderingState< Vec3, Float >::getViewProjection() const
	{
		//Matrix4f view = Matrix4f::Identity();
		//view.block( 0, 0, 3, 4 ) = m_trackball->getViewMatrix().block( 0, 0, 3, 4 );
		
		Affine3d view = m_camTrackball->getViewMatrix();
		Matrix4f proj = m_camTrackball->getProjectionMatrix();
		
		return proj * view;
	}
}

#endif