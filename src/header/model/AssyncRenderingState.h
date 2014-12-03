#ifndef ASSYNC_RENDERING_STATE
#define ASSYNC_RENDERING_STATE

#include "RenderingState.h"

namespace model
{
	/** RenderingState optimized by assync buffer transfers ( OpenGL Insights, section 28.3.3:
	 * @link http://www.seas.upenn.edu/~pcozzi/OpenGLInsights/OpenGLInsights-AsynchronousBufferTransfers.pdf ). */
	template< typename Vec3 >
	class AssyncRenderingState
	: RenderingState< Vec3 >
	{
	public:
		AssyncRenderingState( QGLPainter* painter, const Attributes& attribs );
		~AssyncRenderingState();
		unsigned int renderAssync();
	private:
		int currBuffer;
		const static int N_BUFFERS = 3;
		GLuint buffers[ N_BUFFERS ];
		glSync fences[ N_BUFFERS ];
	};
	
	template< typename Vec3 >
	AssyncRenderingState< Vec3 >::AssyncRenderingState( const Attributes& attribs )
	: RenderingState::RenderingState( painter, attribs )
	{
		glGenBuffers( N_BUFFERS, buffers );
	}
	
	template< typename Vec3 >
	AssyncRenderingState< Vec3 >::~AssyncRenderingState()
	{
		for( GLuint buffer : buffers )
		{
			glInvalidateBufferData( buffer );
		} 
	}
	
	template< typename Vec3 >
	unsigned int AssyncRenderingState< Vec3 >::renderAssync()
	{
		const int buffer = currBuffer++ % N_BUFFERS;
		
		// Wait until buffer is free to use, in most cases this should not wait because we are using three buffers in chain,
		// glClientWaitSync function can be used for check if the TIMEOUT is zero
		GLenum result = glClientWaitSync( fences[ buffer ], 0, TIMEOUT );
		
		if( result == GL_TIMEOUT_EXPIRED || result == GL_WAIT_FAILED )
		{
			throw runtime_error( "Unexpected result of glClientWaitSync()." );
		}
		
		glDeleteSync( fences[ buffer ] );
		glBindBuffer( GL_ARRAY_BUFFER, buffers[ buffer ]);
		void *ptr = glMapBufferRange( GL_ARRAY_BUFFER, offset, size, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT );

		// Fill ptr with useful data
		glUnmapBuffer( GL_ARRAY_BUFFER );
		
		// Use buffer in draw operation
		glDrawArray(  );

		// Put fence into command queue
		fences[ buffer ] = glFenceSync( GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	}
}

#endif