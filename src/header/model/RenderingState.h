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
	
	/** Transient rendering related data used while traversing octree. USAGE: call handleNodeRendering()
	 * to indicate that the contents of an octree node should be rendered. After all nodes are issued for
	 * rendering, call render() to render them all.
	 * @param Vec3 is the type for 3-dimensional vector. */
	template< typename Vec3 >
	class RenderingStateBase
	{
	public:
		RenderingStateBase( QGLPainter* painter, const Attributes& attribs );
		
		/** Renders the current state. Should be called at the end of the traversal, when all rendered nodes have
		 * been already handled. */
		void render();
		
		/** Indicates that the node contents passed should be rendered.
		 * @param NodeContents is the type of nodes' contents. The octree is aware of this type.
		 */
		template< typename NodeContents >
		void handleNodeRendering( const NodeContents& contents );
		
		QGLPainter* getPainter() { return m_painter; };
		vector< Vec3 >& getPositions() { return m_positions; };
		vector< Vec3 >& getColors() { return m_colors; };
		vector< Vec3 >& getNormals() { return m_normals; };
	
	private:
		QGLPainter* m_painter;
		vector< Vec3 > m_positions;
		vector< Vec3 > m_colors;
		vector< Vec3 > m_normals;
		Attributes m_attribs;
	};
	
	template< typename Vec3 >
	RenderingStateBase< Vec3 >::RenderingStateBase( QGLPainter* painter, const Attributes& attribs )
	: m_painter( painter ),
	m_attribs( attribs )
	{
		m_painter->clearAttributes();
		
		switch( m_attribs )
		{
			case Attributes::NORMALS:
			{
				m_painter->setStandardEffect( QGL::LitMaterial );
				break;
			}
			case Attributes::COLORS:
			{
				m_painter->setStandardEffect( QGL::FlatPerVertexColor );
				break;
			}
			case Attributes::COLORS_AND_NORMALS:
			{
				throw logic_error( "Colors and normals not supported yet." );
			}
		}
	}
	
	template< typename Vec3 >
	void RenderingStateBase< Vec3 >::render()
	{
		// TODO: Find a way to specify the precision properly here,
		QGLAttributeValue pointValues( 3, GL_FLOAT, 0, &m_positions[0] );
		QGLAttributeValue colorValues( 3, GL_FLOAT, 0, &m_colors[0] );
		m_painter->setVertexAttribute( QGL::Position, pointValues );
		
		switch( m_attribs )
		{
			case Attributes::NORMALS:
			{
				m_painter->setVertexAttribute( QGL::Normal, colorValues );
				break;
			}
			case Attributes::COLORS:
			{
				m_painter->setVertexAttribute( QGL::Color, colorValues );
			}
			case Attributes::COLORS_AND_NORMALS:
			{
				QGLAttributeValue normalValues( 3, GL_FLOAT, 0, &m_normals[0] );
				m_painter->setVertexAttribute( QGL::Color, colorValues );
				m_painter->setVertexAttribute( QGL::Normal, normalValues );
			}
		}
		
		m_painter->draw( QGL::Points, m_positions.size() );
	}
	
	template< typename Vec3 >
	class RenderingState
	: public RenderingStateBase< Vec3 >
	{
		RenderingState( QGLPainter* painter, const Attributes& attribs ) : RenderingStateBase( painter, attribs );
		
		template<>
		void handleNodeRendering< PointPtr< float, vec3 > >( const PointPtr& contents );
	};
	
	template< typename Vec3 >
	class RenderingState
	: public RenderingStateBase< Vec3 >
	{
		RenderingState( QGLPainter* painter, const Attributes& attribs );
		
		template<>
		void handleNodeRendering< PointVectorPtr< float, vec3 > >( const PointVectorPtr& contents );
	};
	
	template< typename Vec3 >
	class RenderingState
	: public RenderingStateBase< Vec3 >
	{
		RenderingState( QGLPainter* painter, const Attributes& attribs );
		
		template<>
		void handleNodeRendering< ExtendedPointPtr< float, vec3 > >( const ExtendedPointPtr& contents );
	};
	
	template< typename Vec3 >
	class RenderingState
	: public RenderingStateBase< Vec3 >
	{
		RenderingState( QGLPainter* painter, const Attributes& attribs );
		
		template<>
		void handleNodeRendering< ExtendedPointVectorPtr< float, vec3 > >( const ExtendedPointVectorPtr& contents );
	};
	
	
	template< typename Vec3 >
	inline void RenderingState< Vec3 >::handleNodeRendering< PointPtr< float, vec3 > >(
		const PointPtr< float, vec3 >& point )
	{
		RenderingStateBase::m_positions.push_back( *point->getPos() );
		RenderingStateBase::m_colors.push_back( *point->getColor() );
	}
	
	template< typename Vec3 >
	inline void RenderingState< Vec3 >::handleNodeRendering< PointVectorPtr< float, vec3 > >(
		const PointVectorPtr& points )
	{
		for( PointPtr point : *points )
		{
			RenderingStateBase::m_positions.push_back( *point->getPos() );
			RenderingStateBase::m_colors.push_back( *point->getColor() );
		}
	}
	
	template< typename Vec3 >
	inline void RenderingState< Vec3 >::handleNodeRendering< ExtendedPointPtr< float, vec3 > >(
		const ExtendedPointPtr& point )
	{
		RenderingStateBase::m_positions.push_back( *point->getPos() );
		RenderingStateBase::m_colors.push_back( *point->getColor() );
		RenderingStateBase::m_normals.push_back( *point->getNormal() );
	}
	
	template< typename Vec3 >
	inline void RenderingState< Vec3 >::handleNodeRendering< ExtendedPointVectorPtr< float, vec3 > >(
		const ExtendedPointVectorPtr& points )
	{
		for( ExtendedPointPtr point : *points )
		{
			RenderingStateBase::m_positions.push_back( *point->getPos() );
			RenderingStateBase::m_colors.push_back( *point->getColor() );
			RenderingStateBase::m_normals.push_back( *point->getNormal() );
		}
	}
}

#endif