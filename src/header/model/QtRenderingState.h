#ifndef QT_RENDERING_STATE_H
#define QT_RENDERING_STATE_H

#include "RenderingState.h"
#include <QGLPainter>

namespace model
{
	/** RenderingState using Qt as render. USAGE: in addition to the RenderingState steps, setPainter() should be
	 * called when the QGLPainter is known and everytime it needs to be updated.
	 * @param Vec3 is the type for 3-dimensional vector.
	 * @param Float is the type for floating point numbers. */
	template< typename Vec3, typename Float >
	class QtRenderingState
	: public RenderingState< Vec3, Float >
	{
		using RenderingState = model::RenderingState< Vec3, Float >;
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
	
	protected:
		QVector2D projToWindowCoords( const QVector4D& point, const QMatrix4x4& viewProj ) const;
		
		QGLPainter* m_painter;
		QVector2D m_viewportSize;
	};
	
	template< typename Vec3, typename Float >
	QtRenderingState< Vec3, Float >::~QtRenderingState() {}
	
	template< typename Vec3, typename Float >
	inline bool QtRenderingState< Vec3, Float >::isCullable( const pair< Vec3, Vec3 >& box ) const
	{
		Vec3 minBoxVert = box.first;
		Vec3 maxBoxVert = box.second;
		QBox3D qBox( QVector3D( minBoxVert.x, minBoxVert.y, minBoxVert.z ),
					 QVector3D( maxBoxVert.x, maxBoxVert.y, maxBoxVert.z ) );
		
		return m_painter->isCullable( qBox );
	}
	
	template< typename Vec3, typename Float >
	inline bool QtRenderingState< Vec3, Float >::isRenderable( const pair< Vec3, Vec3 >& box,
															   const Float& projThresh ) const
	{
		Vec3 rawMin = box.first;
		Vec3 rawMax = box.second;
		QVector4D min( rawMin.x, rawMin.y, rawMin.z, 1 );
		QVector4D max( rawMax.x, rawMax.y, rawMax.z, 1 );
		
		QGLPainter* painter = m_painter;
		QMatrix4x4 viewProj = painter->combinedMatrix();
		
		QVector2D proj0 = projToWindowCoords( min, viewProj );
		QVector2D proj1 = projToWindowCoords( max, viewProj );
		
		QVector2D diagonal0 = proj1 - proj0;
		
		Vec3 rawSize = rawMax - rawMin;
		QVector3D boxSize( rawSize.x, rawSize.y, rawSize.z );
		
		proj0 = projToWindowCoords( QVector4D( min.x() + boxSize.x(), min.y() + boxSize.y(), min.z(), 1 ), viewProj );
		proj1 = projToWindowCoords( QVector4D( max.x(), max.y(), max.z() + boxSize.z(), 1 ), viewProj );
		
		QVector2D diagonal1 = proj1 - proj0;
		
		Float maxDiagLength = glm::max( diagonal0.lengthSquared(), diagonal1.lengthSquared() );
		
		return maxDiagLength < projThresh;
	}
	
	template< typename Vec3, typename Float >
	void QtRenderingState< Vec3, Float >::setPainter( QGLPainter* painter, const QSize& viewportSize )
	{
		m_painter = painter;
		m_viewportSize.setX( viewportSize.width() );
		m_viewportSize.setY( viewportSize.height() );
	}
	
	template< typename Vec3, typename Float >
	inline QVector2D QtRenderingState< Vec3, Float >::projToWindowCoords( const QVector4D& point, const QMatrix4x4& viewProj )
	const
	{
		QVector4D proj = viewProj.map( point );
		QVector2D normalizedProj( proj / proj.w() );
		//QVector2D windowProj = ( normalizedProj + QVector2D( 1.f, 1.f ) ) * 0.5f * m_viewportSize;
		//return windowProj;
		return normalizedProj;
	}
}

#endif