#ifndef TRANSIENT_RENDERING_STATE_H
#define TRANSIENT_RENDERING_STATE_H

#include "QtRenderingState.h"

namespace model
{
	/** Transient rendering state. Transient because it is designed to be instatiated and used in a single rendering loop
	 * iteration. */
	
	class TransientRenderingState
	: public QtRenderingState
	{
	public:
		TransientRenderingState( QGLPainter* painter, const QSize& viewportSize );
		~TransientRenderingState() {}
		
		unsigned int render();
	};
}

#endif