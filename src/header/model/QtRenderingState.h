#ifndef QT_RENDERING_STATE_H
#define QT_RENDERING_STATE_H

#include "RenderingState.h"
#include "MortonCode.h"
#include "OctreeNode.h"
#include <QGLPainter>

namespace model
{
	/** RenderingState using Qt as render. USAGE: in addition to the RenderingState steps, setPainter() should be
	 * called when the QGLPainter is known and everytime it needs to be updated. */
	class QtRenderingState
	: public RenderingState
	{
	public:
		QtRenderingState( const Attributes& attribs ) : RenderingState( attribs ) {  }
		
		virtual ~QtRenderingState() = 0;
		
		QGLPainter* getPainter() { return m_painter; };
		
		/** This method should be called on the starting of the rendering loop, when the painter is known. */
		void setPainter( QGLPainter* painter, const QSize& viewportSize );
		
		virtual bool isCullable( const pair< Vec3, Vec3 >& box ) const;
		
		/** This implementation will compare the size of the maximum box diagonal in window coordinates with the projection
		 * threshold.
		 *	@param projThresh is the threshold of the squared size of the maximum box diagonal in window coordinates. */
		virtual bool isRenderable( const pair< Vec3, Vec3 >& box, const Float& projThresh ) const;
		
		/** Draws the boundaries of the octree nodes.
		 * @param passProjTestOnly indicates if only the nodes that pass the projection test should be rendered. */
		template< typename Octree, typename MortonCode >
		void drawBoundaries( const Octree& octree, const bool& passProjTestOnly, const Float& projThresh ) const;
		
		/** Utility method to insert node boundary point into vectors for rendering. */
		static void insertBoundaryPoints( vector< Vec3 >& verts, vector< Vec3 >& colors,
										  const pair< Vec3, Vec3 >& box, const bool& isCullable,
									const bool& isRenderable );
	
	protected:
		QVector2D projToWindowCoords( const QVector4D& point, const QMatrix4x4& viewProj ) const;
		
		QGLPainter* m_painter;
		QVector2D m_viewportSize;
	};
}

#endif