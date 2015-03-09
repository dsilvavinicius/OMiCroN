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
		void setPainter( QGLPainter* painter ) { m_painter = painter; }
		
		virtual bool isCullable( const pair< Vec3, Vec3 >& box ) const;
		
		virtual bool isRenderable( const pair< Vec3, Vec3 >& box, const Float& projThresh ) const;
	
	protected:
		QGLPainter* m_painter;
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
		QMatrix4x4 modelViewProjection = painter->combinedMatrix();
		
		QVector4D proj0 = modelViewProjection.map( min );
		QVector2D normalizedProj0( proj0 / proj0.w() );
		
		QVector4D proj1 = modelViewProjection.map( max );
		QVector2D normalizedProj1( proj1 / proj1.w() );
		
		QVector2D diagonal0 = normalizedProj1 - normalizedProj0;
		
		Vec3 rawSize = rawMax - rawMin;
		QVector3D boxSize( rawSize.x, rawSize.y, rawSize.z );
		
		proj0 = modelViewProjection.map( QVector4D(min.x() + boxSize.x(), min.y() + boxSize.y(), min.z(), 1) );
		normalizedProj0 = QVector2D( proj0 / proj0.w() );
		
		proj1 = modelViewProjection.map( QVector4D(max.x(), max.y(), max.z() + boxSize.z(), 1) );
		normalizedProj1 = QVector2D( proj1 / proj1.w() );
		
		QVector2D diagonal1 = normalizedProj1 - normalizedProj0;
		
		Float maxDiagLength = glm::max( diagonal0.lengthSquared(), diagonal1.lengthSquared() );
		
		return maxDiagLength < projThresh;
	}
}

#endif