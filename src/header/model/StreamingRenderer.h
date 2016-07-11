#ifndef STREAMING_RENDERER_H
#define STREAMING_RENDERER_H

#include <mesh.hpp>
#include "TucanoRenderingState.h"
#include "OglUtils.h"

// #define DEBUG

using namespace Tucano;
using namespace util;

namespace model
{
	/**
	 * Renderer which defines a mesh at initalization time and streams points into it as needed in order to update parts
	 * of it. The buffer rendering is controlled by an index buffer. The vertex attributes not referenced in the index
	 * buffer have trash and accessing it can result in undefined behavior. Support for segmentated update is provided. The
	 * workflow to follow is:
	 * 1) Select a segment to update with selectFirstSegment() or selectNextSegment.
	 * 2) Maps the vertex attributes of the current segment with mapAttribs().
	 * 2) Stream points to the segment with handleNodeRendering(). The points will be inserted sequentially, starting at
	 * buffer beginning and, thus, overwriting any previous data. If more than the maximum number of points allowed per
	 * segment is reached, the next points will be ignored.
	 * 3) render(). This will unmap all vertex attributes.
	 * The class also provides API for camera culling and box projection threshold evaluation.
	 */
	template< typename Point >
	class StreamingRenderer
	{
	public:
		using PointPtr = shared_ptr< Point >;
		using PointArray = Array< PointPtr >;
		
		enum Effect
		{
			PHONG,
			JUMP_FLOODING
		};
		
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
		
		~StreamingRenderer();
		
		/** Selects the first segment. To map all pointers to its vertex attributes, call mapAttribs afterwards. */
		void selectFirstSegment();
		
		/** Increments the current segment index in a round-robin fashion. To map all pointers to its vertex
		 * attributes, call mapAttribs afterwards. */
		void selectNextSegment();
		
		/** @returns the index of the current segment. */
		int currentSegment() { return m_currentSegment; }
		
		/** Maps all vertex attributes of the current segment. Future point rendering will overwrite this segment previous
		 * data. */
		void mapAttribs();
		
		/** Event ocurring to setup rendering. Must be called before handling any node in a rendering loop.
		 * Default implementation does nothing. */
		void setupRendering();
		
		/** Renders the current state. Should be called after handling all nodes to be rendered. Unmaps all vertex
		 * attributes.
		 * @returns the number of rendered points. */
		uint render();
		
		/** Indicates that the node contents passed should be rendered. */
		void handleNodeRendering( const PointArray& points );
		
		/** Updates the frustum after changes on camera. */
		void update();
		
		bool isCullable( const AlignedBox3f& box ) const;
		
		/** This implementation will compare the size of the maximum box diagonal in window coordinates with the projection
		 *	threshold.
		 *	@param projThresh is the threshold of the squared size of the maximum box diagonal in window coordinates. */
		bool isRenderable( const AlignedBox3f& box, const Float projThresh ) const;
		
		/** Gets the image space pbr effect. The caller is reponsable for the correct usage.*/
		ImgSpacePBR& getJumpFlooding() { return *m_jfpbr; }
		
		/** Gets the phong effect. The caller is reponsable for the correct usage.*/
		Phong& getPhong() { return *m_phong; }
		
		/** Changes the effect used to render the points. */
		void selectEffect( const Effect& effect ) { m_effect = effect; }
		
		void setJfpbrFrameskip( const int& value ) { m_jfpbrFrameskip = value; }
	
	private:
		void initColors( const uint totalPoints );
		void unmapColors();
		
		/** Acquires current traball's view-projection matrix. */
		void updateViewProjection();
		
		/** Projects the point in world coordinates to window coordinates. */
		Vector2i projToWindowCoords( const Vector4f& point, const Matrix4f& viewProjection, const Vector2i& viewportSize )
		const;
		
		Frustum* m_frustum;
		Camera* m_camera;
		Camera* m_lightCamera;
		
		Matrix4f m_viewProj;
		
		Mesh* m_mesh;
		Phong* m_phong;
		ImgSpacePBR *m_jfpbr;
		
		Effect m_effect;
		
		/** Frameskip for the Jump Flooding effect. */
		int m_jfpbrFrameskip;
		
		/** Frame counter. Used in order to skip frames properly. */
		unsigned int m_nFrames;
		
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
	: m_camera( camera ),
	m_lightCamera( lightCam ),
	m_mesh( mesh ),
	m_jfpbrFrameskip( jfpbrFrameskip ),
	m_effect( effect ),
	m_nFrames( 0 ),
	m_ptsPerSegment( nSegments ),
	m_maxPtsPerSegment( nPointsPerSegment ),
	m_nTotalPoints( 0 ),
	m_currentSegment( 0 )
	{
		uint totalPoints = m_maxPtsPerSegment * nSegments;
		
		m_mesh->reset();
		m_mesh->reserveVertices( 3, totalPoints );
		m_mesh->reserveNormals( totalPoints );
		initColors( totalPoints );
		m_mesh->reserveIndices( totalPoints );
		
		mesh->selectPrimitive( Mesh::POINT );
		
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
	}
	
	template< typename Point >
	inline StreamingRenderer< Point >::~StreamingRenderer()
	{
		delete m_jfpbr;
		delete m_phong;
		delete m_frustum;
	}
	
	template< typename Point >
	void StreamingRenderer< Point >::update()
	{
		updateViewProjection();
		m_frustum->update( m_viewProj );
	}
	template< typename Point >
	void StreamingRenderer< Point >::selectFirstSegment()
	{
		m_currentSegment = 0;
	}
	
	template< typename Point >
	void StreamingRenderer< Point >::selectNextSegment()
	{
		m_currentSegment = ( m_currentSegment + 1 ) % m_ptsPerSegment.size();
	}
	
	template<>
	inline void StreamingRenderer< Point >::mapAttribs()
	{
		m_nTotalPoints -= m_ptsPerSegment[ m_currentSegment ];
		m_ptsPerSegment[ m_currentSegment ] = 0ul;
		
		m_vertexMap = m_mesh->mapVertices( m_currentSegment * m_maxPtsPerSegment, m_maxPtsPerSegment );
		m_normalMap = m_mesh->mapNormals( m_currentSegment * m_maxPtsPerSegment, m_maxPtsPerSegment );
	}
	
	template<>
	inline void StreamingRenderer< ExtendedPoint >::mapAttribs()
	{
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
		
		glEnable( GL_DEPTH_TEST );
		
		glPointSize( 2 );
		
		update();
	}
	
	template< typename Point >
	inline bool StreamingRenderer< Point >::isCullable( const AlignedBox3f& box ) const
	{
		return m_frustum->isCullable( box );
	}
	
	template< typename Point >
	inline bool StreamingRenderer< Point >::isRenderable( const AlignedBox3f& box, const Float projThresh ) const
	{
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
	
	template<>
	inline void StreamingRenderer< Point >::handleNodeRendering( const PointArray& points )
	{
		for( const PointPtr p : points )
		{
			const Vec3& pos = p->getPos();
			const Vec3& normal = p->getColor();
			
			*( m_vertexMap++ ) = pos.x();
			*( m_vertexMap++ ) = pos.y();
			*( m_vertexMap++ ) = pos.z();
			
			*( m_normalMap++ ) = normal.x();
			*( m_normalMap++ ) = normal.y();
			*( m_normalMap++ ) = normal.z();
		}
		
		m_ptsPerSegment[ m_currentSegment ] += points.size();
	}
	
	template<>
	inline void StreamingRenderer< ExtendedPoint >::handleNodeRendering( const PointArray& points )
	{
		for( const ExtendedPointPtr p : points )
		{
			const Vec3& pos = p->getPos();
			const Vec3& normal = p->getNormal();
			const Vec3& color = p->getColor();
			
			*( m_vertexMap++ ) = pos.x();
			*( m_vertexMap++ ) = pos.y();
			*( m_vertexMap++ ) = pos.z();
			
			*( m_normalMap++ ) = normal.x();
			*( m_normalMap++ ) = normal.y();
			*( m_normalMap++ ) = normal.z();
			
			*( m_colorMap++ ) = color.x();
			*( m_colorMap++ ) = color.y();
			*( m_colorMap++ ) = color.z();
		}
		
		m_ptsPerSegment[ m_currentSegment ] += points.size();
	}
	
	template< typename Point >
	inline uint StreamingRenderer< Point >::render()
	{
		#ifdef DEBUG
			cout << "Current segment: " << m_currentSegment << endl << endl;
		#endif
		
		m_nTotalPoints += m_ptsPerSegment[ m_currentSegment ];
		
		m_mesh->unmapVertices();
		
		#ifdef DEBUG
			OglUtils::checkOglErrors();
		#endif
		
		m_mesh->unmapNormals();
		
		#ifdef DEBUG
			OglUtils::checkOglErrors();
		#endif
		
		unmapColors();
		
		#ifdef DEBUG
			OglUtils::checkOglErrors();
		#endif
		
		if( m_nTotalPoints > 0 )
		{
			uint* indicesPtr = m_mesh->mapIndices( 0, m_nTotalPoints );
		
			#ifdef DEBUG
				OglUtils::checkOglErrors();
			#endif
			
			uint prefix = 0u;
			for( int i = 0; i < m_ptsPerSegment.size(); ++i )
			{
				uint nPoints = m_ptsPerSegment[ i ];
				
				#ifdef DEBUG
				{
					cout << "Segment " << i << " points: " << nPoints << endl << endl;
				}
				#endif
				
				#pragma omp parallel for
				for( uint j = 0; j < nPoints; ++j )
				{
					indicesPtr[ prefix + j ] = i * m_maxPtsPerSegment + j;
				}
				prefix += nPoints ;
			}
			
			m_mesh->unmapIndices();
			
			#ifdef DEBUG
				OglUtils::checkOglErrors();
			#endif
				
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
			
			#ifdef DEBUG
				OglUtils::checkOglErrors();
			#endif
		}
		
		#ifdef DEBUG
		{
			cout << "Total points: " << m_nTotalPoints << endl << endl;
		}
		#endif
		
		return m_nTotalPoints;
	}
	
	template< typename Point >
	inline void StreamingRenderer< Point >::updateViewProjection()
	{
		Matrix4f view = m_camera->getViewMatrix().matrix();
		Matrix4f proj = m_camera->getProjectionMatrix();
		
		m_viewProj = proj * view;
	}
	
	template< typename Point >
	inline Vector2i StreamingRenderer< Point >
	::projToWindowCoords( const Vector4f& point, const Matrix4f& viewProj, const Vector2i& viewportSize ) const
	{
		Vector4f proj = viewProj * point;
		return Vector2i( ( proj.x() / proj.w() + 1.f ) * 0.5f * viewportSize.x(),
						 ( proj.y() / proj.w() + 1.f ) * 0.5f * viewportSize.y() );
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

#undef DEBUG

#endif