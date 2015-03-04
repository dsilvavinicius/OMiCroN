#ifndef TRANSIENT_RENDERING_STATE_H
#define TRANSIENT_RENDERING_STATE_H

#include "QtRenderingState.h"

namespace model
{
	/** Transient rendering state. Transient because it is designed to be instatiated and used in a single rendering loop
	 * iteration. */
	template< typename Vec3, typename Float >
	class TransientRenderingState
	: public QtRenderingState< Vec3, Float >
	{
		using RenderingState = model::RenderingState< Vec3, Float >;
		using QtRenderingState = model::QtRenderingState< Vec3, Float >;
	public:
		TransientRenderingState( QGLPainter* painter, const Attributes& attribs );
		~TransientRenderingState() {}
		
		unsigned int render();
	};
	
	template< typename Vec3, typename Float >
	TransientRenderingState< Vec3, Float >::TransientRenderingState( QGLPainter* painter,
																	 const Attributes& attribs )
	: QtRenderingState( attribs )
	{
		QtRenderingState::setPainter( painter );
		QtRenderingState::m_painter->clearAttributes();
		
		switch( RenderingState::m_attribs )
		{
			case Attributes::NORMALS:
			{
				QtRenderingState::m_painter->setStandardEffect( QGL::LitMaterial );
				break;
			}
			case Attributes::COLORS:
			{
				QtRenderingState::m_painter->setStandardEffect( QGL::FlatPerVertexColor );
				break;
			}
			case Attributes::COLORS_AND_NORMALS:
			{
				throw logic_error( "Colors and normals not supported yet." );
				break;
			}
		}
	}
	
	template< typename Vec3, typename Float >
	unsigned int TransientRenderingState< Vec3, Float >::render()
	{
		// TODO: Find a way to specify the precision properly here,
		QGLAttributeValue pointValues( 3, GL_FLOAT, 0, &RenderingState::m_positions[0] );
		QGLAttributeValue colorValues( 3, GL_FLOAT, 0, &RenderingState::m_colors[0] );
		QtRenderingState::m_painter->setVertexAttribute( QGL::Position, pointValues );
		
		switch( RenderingState::m_attribs )
		{
			case Attributes::NORMALS:
			{
				QtRenderingState::m_painter->setVertexAttribute( QGL::Normal, colorValues );
				break;
			}
			case Attributes::COLORS:
			{
				QtRenderingState::m_painter->setVertexAttribute( QGL::Color, colorValues );
				break;
			}
			case Attributes::COLORS_AND_NORMALS:
			{
				QGLAttributeValue normalValues( 3, GL_FLOAT, 0, &RenderingState::m_normals[0] );
				QtRenderingState::m_painter->setVertexAttribute( QGL::Color, colorValues );
				QtRenderingState::m_painter->setVertexAttribute( QGL::Normal, normalValues );
				break;
			}
		}
		
		unsigned int numRenderedPoints = RenderingState::m_positions.size();
		QtRenderingState::m_painter->draw( QGL::Points, numRenderedPoints );
		
		return numRenderedPoints;
	}
}

#endif