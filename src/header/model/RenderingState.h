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
	class RenderingState
	{
	public:
		RenderingState( QGLPainter* painter, const Attributes& attribs );
		
		/** Renders the current state. Should be called at the end of the traversal, when all rendered nodes have
		 * been already handled.
		 * @returns the number of rendered points. */
		unsigned long render();
		
		/** Indicates that the node contents passed should be rendered. A static method was used to overcome C++ limitation of
		* class member specializations.
		* @param NodeContents is the type of nodes' contents. The octree is aware of this type.
		*/
		template< typename NodeContents >
		void handleNodeRendering( RenderingState< Vec3 >& state, const NodeContents& contents );
		
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
	RenderingState< Vec3 >::RenderingState( QGLPainter* painter, const Attributes& attribs )
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
				break;
			}
		}
	}
	
	template< typename Vec3 >
	unsigned long RenderingState< Vec3 >::render()
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
				break;
			}
			case Attributes::COLORS_AND_NORMALS:
			{
				QGLAttributeValue normalValues( 3, GL_FLOAT, 0, &m_normals[0] );
				m_painter->setVertexAttribute( QGL::Color, colorValues );
				m_painter->setVertexAttribute( QGL::Normal, normalValues );
				break;
			}
		}
		
		unsigned long numRenderedPoints = m_positions.size();
		m_painter->draw( QGL::Points, numRenderedPoints );
		
		return numRenderedPoints;
	}
	
	namespace NodeRenderingHandler
	{
		template< typename Vec3, typename NodeContents >
		void handle( RenderingState< Vec3 >& state, const NodeContents& contents );
		
		template<>
		inline void handle< vec3, PointPtr< float, vec3 > >( RenderingState< vec3 >& state,
															 const PointPtr< float, vec3 >& point )
		{
			state.getPositions().push_back( *point->getPos() );
			state.getColors().push_back( *point->getColor() );
		}
	
		template<>
		inline void handle< vec3, PointVectorPtr< float, vec3 > >( RenderingState< vec3 >& state,
																   const PointVectorPtr< float, vec3 >& points )
		{
			for( PointPtr< float, vec3 > point : *points )
			{
				state.getPositions().push_back( *point->getPos() );
				state.getColors().push_back( *point->getColor() );
			}
		}
	
		template<>
		inline void handle< vec3, ExtendedPointPtr< float, vec3 > >( RenderingState< vec3 >& state,
																		 const ExtendedPointPtr< float, vec3 >& point )
		{
			state.getPositions().push_back( *point->getPos() );
			state.getColors().push_back( *point->getColor() );
			state.getNormals().push_back( *point->getNormal() );
		}
	
		template<>
		inline void handle< vec3, ExtendedPointVectorPtr< float, vec3 > >( RenderingState< vec3 >& state,
																		const ExtendedPointVectorPtr< float, vec3 >& points )
		{
			for( ExtendedPointPtr< float, vec3 > point : *points )
			{
				state.getPositions().push_back( *point->getPos() );
				state.getColors().push_back( *point->getColor() );
				state.getNormals().push_back( *point->getNormal() );
			}
		}
	}
	
	template< typename Vec3 >
	template< typename NodeContents >
	void RenderingState< Vec3 >::handleNodeRendering( RenderingState& state, const NodeContents& contents )
	{
		NodeRenderingHandler::handle< Vec3, NodeContents >( state, contents );
	}
}

#endif