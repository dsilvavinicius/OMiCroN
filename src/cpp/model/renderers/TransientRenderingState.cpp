#include "renderers/TransientRenderingState.h"

namespace model
{
	TransientRenderingState::TransientRenderingState( QGLPainter* painter, const QSize& viewportSize )
	: QtRenderingState()
	{
		QtRenderingState::setPainter( painter, viewportSize );
		QtRenderingState::m_painter->clearAttributes();
		QtRenderingState::m_painter->setStandardEffect( QGL::LitMaterial );
	}
	
	unsigned int TransientRenderingState::render()
	{
		// TODO: Find a way to specify the precision properly here,
		QGLAttributeValue pointValues( 3, GL_FLOAT, 0, &RenderingState::m_positions[0] );
		QGLAttributeValue normalValues( 3, GL_FLOAT, 0, &RenderingState::m_normals[0] );
		QGLAttributeValue colorValues( 3, GL_FLOAT, 0, &RenderingState::m_colors[0] );
		
		QtRenderingState::m_painter->setVertexAttribute( QGL::Position, pointValues );
		QtRenderingState::m_painter->setVertexAttribute( QGL::Normal, normalValues );
		QtRenderingState::m_painter->setVertexAttribute( QGL::Color, colorValues );
		
		unsigned int numRenderedPoints = RenderingState::m_positions.size();
		QtRenderingState::m_painter->draw( QGL::Points, numRenderedPoints );
		
		return numRenderedPoints;
	}
}