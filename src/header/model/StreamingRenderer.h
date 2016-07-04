#ifndef STREAMING_RENDERER_H
#define STREAMING_RENDERER_H

#include <mesh.hpp>
#include "TucanoRenderingState.h"

using namespace Tucano;

namespace model
{
	/**
	 * Renderer which defines a mesh at initalization time and streams points into it as needed in order to update parts
	 * of it. The buffer rendering is controlled by an index buffer. The vertex attributes not referenced in the index
	 * buffer have trash and accessing it can result in undefined behavior. Support for segmentated update is provided. The
	 * workflow to follow is:
	 * 1) Select a segment to update with selectSegment().
	 * 2) Stream points to the segment with handleNodeRendering(). The points will be inserted sequentially. If more than
	 * the maximum number of points allowed per segment is reached, the next points will be ignored.
	 * 3) render().
	 * The class also provides API for camera culling and box projection threshold evaluation.
	 */
	template< typename Point >
	class StreamingRenderer
	: public TucanoRenderingState
	{
	public:
		using PointPtr = shared_ptr< Point >;
		using PointArray = Array< PointPtr >;
		
		/**
		 * Ctor. Takes control of a Mesh, instancing the streaming buffers inside it. The initial mesh has just trash. The
		 * caller must ensure that the mesh will have lifetime greater than this StreamingRenderer.
		 * @param nPointsPerSegment is the maximum mesh segment size.
		 * @param nSegments is the number of segments.
		 * @mesh is a pointer to the mesh that will be controlled by the renderer.
		 */
		StreamingRenderer( Camera* camera, Camera* lightCam , Mesh* mesh, const string& shaderPath,
						   const uint nPointsPerSegment, const int nSegments, const int& jfpbrFrameskip = 1,
					 const Effect& effect = PHONG );
		
		~StreamingRenderer() {}
		
		/** Selects the segment to insert points and maps all pointer to its vertex attributes. */
		void selectSegment( const int segment );
		
		/** Event ocurring to setup rendering. Must be called before handling any node in a rendering loop.
		 * Default implementation does nothing. */
		void setupRendering() override;
		
		/** Renders the current state. Should be called at the end of the traversal, when all rendered nodes have
		 * been already handled.
		 * @returns the number of rendered points. */
		uint render() override;
		
		/** Indicates that the node contents passed should be rendered. */
		virtual void handleNodeRendering( const PointArray& points ) override;
	
	private:
		void initColors( const uint totalPoints );
		void unmapColors();
		
		/** Number of current points per segment. */
		Array< uint > m_ptsPerSegment;
		
		/** Current segment vertex map. */
		float* m_vertexMap;
		/** Current segment normal map. */
		float* m_normalMap;
		/** Current segment color map. */
		float* m_colorMap;
		
		/** Maximum number of points allowed in a segment. */
		uint m_maxPtsPerSegment;
		
		/** Number of all current points. In other words, the sum of the number of points in all segments. */
		uint m_nTotalPoints;
		
		int m_currentSegment;
	};
	
	template< typename Point >
	inline StreamingRenderer< Point >
	::StreamingRenderer( Camera* camera, Camera* lightCam , Mesh* mesh, const string& shaderPath,
						 const uint nPointsPerSegment, const int nSegments, const int& jfpbrFrameskip,
					  const Effect& effect )
	: TucanoRenderingState( camera, lightCam, mesh, shaderPath, jfpbrFrameskip, effect ),
	m_ptsPerSegment( nSegments ),
	m_maxPtsPerSegment( nPointsPerSegment ),
	m_nTotalPoints( 0 ),
	m_currentSegment( 0 )
	{
		uint totalPoints = m_maxPtsPerSegment * nSegments;
		m_indices.resize( totalPoints );
		
		m_mesh->reset();
		m_mesh->reserveVertices( 3, totalPoints );
		m_mesh->reserveNormals( totalPoints );
		initColors( totalPoints );
		
		mesh->selectPrimitive( Mesh::POINT );
	}
	
	template<>
	inline void StreamingRenderer< Point >::selectSegment( const int segment )
	{
		m_currentSegment = segment;
		m_nTotalPoints -= m_ptsPerSegment[ m_currentSegment ];
		m_ptsPerSegment[ m_currentSegment ] = 0ul;
		
		m_vertexMap = m_mesh->mapVertices( m_currentSegment * m_maxPtsPerSegment, m_maxPtsPerSegment );
		m_normalMap = m_mesh->mapNormals( m_currentSegment * m_maxPtsPerSegment, m_maxPtsPerSegment );
	}
	
	template<>
	inline void StreamingRenderer< ExtendedPoint >::selectSegment( const int segment )
	{
		m_currentSegment = segment;
		m_nTotalPoints -= m_ptsPerSegment[ m_currentSegment ];
		m_ptsPerSegment[ m_currentSegment ] = 0ul;
		
		m_vertexMap = m_mesh->mapVertices( m_currentSegment * m_maxPtsPerSegment, m_maxPtsPerSegment );
		m_normalMap = m_mesh->mapNormals( m_currentSegment * m_maxPtsPerSegment, m_maxPtsPerSegment );
		m_colorMap = m_mesh->mapColors( m_currentSegment * m_maxPtsPerSegment, m_maxPtsPerSegment );
	}
	
	template< typename Point >
	inline void StreamingRenderer< Point >::setupRendering()
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
		
		update();
	}
	
	template<>
	void StreamingRenderer< Point >::handleNodeRendering( const PointArray& points )
	{
		float* vertPtr = m_vertexMap;
		float* normalPtr = m_normalMap;
		
		for( const PointPtr p : points )
		{
			const Vec3& pos = p->getPos();
			const Vec3& normal = p->getColor();
			
			*( vertPtr++ ) = pos.x();
			*( vertPtr++ ) = pos.y();
			*( vertPtr++ ) = pos.z();
			
			*( normalPtr++ ) = normal.x();
			*( normalPtr++ ) = normal.y();
			*( normalPtr++ ) = normal.z();
		}
		
		m_ptsPerSegment[ m_currentSegment ] += points.size();
	}
	
	template<>
	void StreamingRenderer< ExtendedPoint >::handleNodeRendering( const PointArray& points )
	{
		float* vertPtr = m_vertexMap;
		float* normalPtr = m_normalMap;
		float* colorPtr = m_colorMap;
		
		for( const ExtendedPointPtr p : points )
		{
			const Vec3& pos = p->getPos();
			const Vec3& normal = p->getNormal();
			const Vec3& color = p->getColor();
			
			*( vertPtr++ ) = pos.x();
			*( vertPtr++ ) = pos.y();
			*( vertPtr++ ) = pos.z();
			
			*( normalPtr )++ = normal.x();
			*( normalPtr )++ = normal.y();
			*( normalPtr )++ = normal.z();
			
			*( colorPtr )++ = color.x();
			*( colorPtr )++ = color.y();
			*( colorPtr )++ = color.z();
		}
		
		m_ptsPerSegment[ m_currentSegment ] += points.size();
	}
	
	template< typename Point >
	uint StreamingRenderer< Point >::render()
	{
		m_nTotalPoints += m_ptsPerSegment[ m_currentSegment ];
		
		m_mesh->unmapVertices();
		m_mesh->unmapNormals();
		unmapColors();
		
		uint prefix = 0u;
		for( int i = 0; i < m_ptsPerSegment.size(); ++i )
		{
			uint nPoints = m_ptsPerSegment[ i ];
			
			#pragma omp parallel for
			for( uint j = 0; j < nPoints; ++j )
			{
				m_indices[ prefix + j ] = i * m_maxPtsPerSegment;
			}
			prefix += nPoints ;
		}
		
		m_indices.resize( m_nTotalPoints );
		m_mesh->loadIndices( m_indices );
		
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
		
		return m_nTotalPoints;
	}
	
	template<>
	inline void StreamingRenderer< Point >::initColors( const uint totalPoints )
	{}
	
	template<>
	inline void StreamingRenderer< ExtendedPoint >::initColors( const uint totalPoints )
	{
		m_mesh->reserveColors( 3, totalPoints );
	}
	
	template<>
	inline void StreamingRenderer< Point >::unmapColors()
	{}
	
	template<>
	inline void StreamingRenderer< ExtendedPoint >::unmapColors()
	{
		m_mesh->unmapColors();
	}
}

#endif