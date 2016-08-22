#ifndef STREAMING_RENDERER_H
#define STREAMING_RENDERER_H

#include <mesh.hpp>
#include "TucanoRenderingState.h"
#include "O1OctreeNode.h"
#include "OglUtils.h"

// #define DEBUG

using namespace Tucano;
using namespace util;

namespace model
{
	/**
	 * Renderer which expects that each node have its own mesh. The class also provides API for camera culling and box
	 * projection threshold evaluation.
	 */
	template< typename Point >
	class StreamingRenderer
	{
	public:
		using PointPtr = shared_ptr< Point >;
		using PointArray = Array< PointPtr >;
		using Node = O1OctreeNode< PointPtr >;
		
		enum Effect
		{
			PHONG,
			JUMP_FLOODING
		};
		
		/**
		 * Ctor.
		 * @param nPointsPerSegment is the maximum mesh segment size.
		 * @param nSegments is the number of segments.
		 */
		StreamingRenderer( Camera* camera, Camera* lightCam, const string& shaderPath, const int& jfpbrFrameskip = 1,
						   const Effect& effect = PHONG );
		
		~StreamingRenderer();
		
		/** Rendering setup. Must be called before handling any node in a rendering loop. */
		void setupRendering();
		
		/** Must be called after handling all nodes expected to be rendered.
		 * @returns the number of rendered points. */
		uint afterRendering();
		
		/** Indicates that the node should be rendered. */
		void handleNodeRendering( const Node& node );
		
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
		/** Acquires current traball's view-projection matrix. */
		void updateViewProjection();
		
		/** Projects the point in world coordinates to window coordinates. */
		Vector2i projToWindowCoords( const Vector4f& point, const Matrix4f& viewProjection, const Vector2i& viewportSize )
		const;
		
		Frustum* m_frustum;
		Camera* m_camera;
		Camera* m_lightCamera;
		
		Matrix4f m_viewProj;
		
		Phong* m_phong;
		ImgSpacePBR *m_jfpbr;
		
		Effect m_effect;
		
		/** Frameskip for the Jump Flooding effect. */
		int m_jfpbrFrameskip;
		
		/** Frame counter. Used in order to skip frames properly. */
		unsigned int m_nFrames;
		
		ulong m_nRenderedPoints;
	};
	
	template< typename Point >
	inline StreamingRenderer< Point >
	::StreamingRenderer( Camera* camera, Camera* lightCam, const string& shaderPath, const int& jfpbrFrameskip,
						 const Effect& effect )
	: m_camera( camera ),
	m_lightCamera( lightCam ),
	m_jfpbrFrameskip( jfpbrFrameskip ),
	m_effect( effect ),
	m_nFrames( 0 ),
	m_nRenderedPoints( 0ul )
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
		m_jfpbr->setFirstMaxDistance( 0.002f );
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
	
	template< typename Point >
	inline void StreamingRenderer< Point >::handleNodeRendering( const Node& node )
	{
		Mesh& mesh = node.mesh();
		if( mesh.getNumberOfVertices() > 0 )
		{
			m_nRenderedPoints += mesh.getNumberOfVertices();
			m_phong->render( mesh, *m_camera, *m_lightCamera );
			
// 			switch( m_effect )
// 			{
// 				case PHONG: m_phong->render( mesh, *m_camera, *m_lightCamera ); break;
// 				case JUMP_FLOODING:
// 				{
// 					bool newFrame = m_nFrames % m_jfpbrFrameskip == 0;
// 					m_jfpbr->render( m_mesh, m_camera, m_lightCamera, newFrame );
// 					
// 					break;
// 				}
// 			}
		}
	}
	
	template< typename Point >
	inline uint StreamingRenderer< Point >::afterRendering()
	{
		ulong renderedPoints = m_nRenderedPoints;
		m_nRenderedPoints = 0ul;
		
		return renderedPoints;
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
}

#undef DEBUG

#endif