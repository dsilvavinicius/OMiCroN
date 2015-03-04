#ifndef RENDERING_STATE_H
#define RENDERING_STATE_H

#include <stdexcept>
#include <vector>
#include <QGLPainter>
#include "ExtendedPoint.h"

using namespace std;

namespace model
{
	enum Attributes
	{
		COLORS = 0x1,
		NORMALS = 0x2,
		COLORS_AND_NORMALS = 0x3
	};
	
	/** Rendering related data used while traversing octree. USAGE: call setPainter() when the painter is known in the
	 * rendering loop and call several handleNodeRendering() afterwards to indicate that the contents of octree nodes should be
	 * rendered. After all nodes are issued for rendering, call render() to render them all.
	 * @param Vec3 is the type for 3-dimensional vector.
	 * @param Float is the type for floating point numbers. */
	template< typename Vec3, typename Float >
	class RenderingState
	{
	public:
		RenderingState( const Attributes& attribs );
		
		/** Renders the current state. Should be called at the end of the traversal, when all rendered nodes have
		 * been already handled.
		 * @returns the number of rendered points. */
		virtual unsigned int render() = 0;
		
		/** Checks if the axis-aligned box is culled by camera frustum.
		 * @returns true if the box should be culled and false otherwise. */
		virtual bool isCullable( const pair< Vec3, Vec3 >& box ) const;
		
		/** Checks if the axis-aligned box is renderable with the current projection threshold.
		 * @returns true if the box has a projection compatible with the current threshold and, thus, should be
		 * rendered. False otherwise (indicating that the traversal should proceed deeper in the hierarchy). */
		virtual bool isRenderable( const pair< Vec3, Vec3 >& box, const Float& projThresh ) const;
		
		/** Indicates that the node contents passed should be rendered. A static method was used to overcome C++ limitation of
		* class member specializations.
		* @param NodeContents is the type of nodes' contents. The octree is aware of this type.
		*/
		template< typename NodeContents >
		void handleNodeRendering( const NodeContents& contents );
		
		QGLPainter* getPainter() { return m_painter; };
		
		/** This method should be called on the starting of the rendering loop, when the painter is known. */
		void setPainter( QGLPainter* painter ) { m_painter = painter; }
		
		vector< Vec3 >& getPositions() { return m_positions; };
		vector< Vec3 >& getColors() { return m_colors; };
		vector< Vec3 >& getNormals() { return m_normals; };
	
	protected:
		QGLPainter* m_painter;
		vector< Vec3 > m_positions;
		vector< Vec3 > m_colors;
		vector< Vec3 > m_normals;
		Attributes m_attribs;
	};
	
	template< typename Vec3, typename Float >
	RenderingState< Vec3, Float >::RenderingState( const Attributes& attribs )
	: m_attribs( attribs ) {}
	
	namespace NodeRenderingHandler
	{
		template< typename Vec3, typename Float, typename NodeContents >
		void handle( RenderingState< Vec3, Float >& state, const NodeContents& contents );
		
		template<>
		inline void handle< vec3, float, PointPtr< float, vec3 > >( RenderingState< vec3, float >& state,
																	const PointPtr< float, vec3 >& point )
		{
			state.getPositions().push_back( *point->getPos() );
			state.getColors().push_back( *point->getColor() );
		}
	
		template<>
		inline void handle< vec3, float, PointVectorPtr< float, vec3 > >( RenderingState< vec3, float >& state,
																	const PointVectorPtr< float, vec3 >& points )
		{
			for( PointPtr< float, vec3 > point : *points )
			{
				state.getPositions().push_back( *point->getPos() );
				state.getColors().push_back( *point->getColor() );
			}
		}
	
		template<>
		inline void handle< vec3, float, ExtendedPointPtr< float, vec3 > >( RenderingState< vec3, float >& state,
																	const ExtendedPointPtr< float, vec3 >& point )
		{
			state.getPositions().push_back( *point->getPos() );
			state.getColors().push_back( *point->getColor() );
			state.getNormals().push_back( *point->getNormal() );
		}
	
		template<>
		inline void handle< vec3, float, ExtendedPointVectorPtr< float, vec3 > >(
			RenderingState< vec3, float >& state, const ExtendedPointVectorPtr< float, vec3 >& points )
		{
			for( ExtendedPointPtr< float, vec3 > point : *points )
			{
				state.getPositions().push_back( *point->getPos() );
				state.getColors().push_back( *point->getColor() );
				state.getNormals().push_back( *point->getNormal() );
			}
		}
	}
	
	template< typename Vec3, typename Float >
	template< typename NodeContents >
	void RenderingState< Vec3, Float >::handleNodeRendering( const NodeContents& contents )
	{
		NodeRenderingHandler::handle< Vec3, Float, NodeContents >( *this, contents );
	}
	
	template< typename Vec3, typename Float >
	inline bool RenderingState< Vec3, Float >::isCullable( const pair< Vec3, Vec3 >& box ) const
	{
		Vec3 minBoxVert = box.first;
		Vec3 maxBoxVert = box.second;
		QBox3D qBox( QVector3D( minBoxVert.x, minBoxVert.y, minBoxVert.z ),
					 QVector3D( maxBoxVert.x, maxBoxVert.y, maxBoxVert.z ) );
		
		return m_painter->isCullable( qBox );
	}
	
	template< typename Vec3, typename Float >
	inline bool RenderingState< Vec3, Float >::isRenderable( const pair< Vec3, Vec3 >& box,
															 const Float& projThresh )
	const
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