#ifndef QT_RENDERING_STATE_H
#define QT_RENDERING_STATE_H

#include "RenderingState.h"

namespace model
{
	/** Transient rendering state. Transient because it is designed to be instatiated and used in a single rendering loop
	 * iteration. */
	template< typename Vec3 >
	class TransientRenderingState
	: public RenderingState< Vec3 >
	{
		using RenderingState = model::RenderingState< Vec3 >;
	public:
		TransientRenderingState( QGLPainter* painter, const Attributes& attribs );
		~TransientRenderingState() {}
		
		unsigned int render();
	};
	
	template< typename Vec3 >
	TransientRenderingState< Vec3 >::TransientRenderingState( QGLPainter* painter, const Attributes& attribs )
	: RenderingState( attribs )
	{
		RenderingState::setPainter( painter );
		RenderingState::m_painter->clearAttributes();
		
		switch( RenderingState::m_attribs )
		{
			case Attributes::NORMALS:
			{
				RenderingState::m_painter->setStandardEffect( QGL::LitMaterial );
				break;
			}
			case Attributes::COLORS:
			{
				RenderingState::m_painter->setStandardEffect( QGL::FlatPerVertexColor );
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
	unsigned int TransientRenderingState< Vec3 >::render()
	{
		// TODO: Find a way to specify the precision properly here,
		QGLAttributeValue pointValues( 3, GL_FLOAT, 0, &RenderingState::m_positions[0] );
		QGLAttributeValue colorValues( 3, GL_FLOAT, 0, &RenderingState::m_colors[0] );
		RenderingState::m_painter->setVertexAttribute( QGL::Position, pointValues );
		
		switch( RenderingState::m_attribs )
		{
			case Attributes::NORMALS:
			{
				RenderingState::m_painter->setVertexAttribute( QGL::Normal, colorValues );
				break;
			}
			case Attributes::COLORS:
			{
				RenderingState::m_painter->setVertexAttribute( QGL::Color, colorValues );
				break;
			}
			case Attributes::COLORS_AND_NORMALS:
			{
				QGLAttributeValue normalValues( 3, GL_FLOAT, 0, &RenderingState::m_normals[0] );
				RenderingState::m_painter->setVertexAttribute( QGL::Color, colorValues );
				RenderingState::m_painter->setVertexAttribute( QGL::Normal, normalValues );
				break;
			}
		}
		
		unsigned int numRenderedPoints = RenderingState::m_positions.size();
		RenderingState::m_painter->draw( QGL::Points, numRenderedPoints );
		
		return numRenderedPoints;
	}
}

#endif