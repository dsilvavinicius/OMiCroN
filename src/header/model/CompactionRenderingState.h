#ifndef COMPACTION_RENDERING_STATE_H
#define COMPACTION_RENDERING_STATE_H

#include <QGLBuffer>
#include <QOpenGLShaderProgram>
#include "RenderingState.h"

namespace model
{
	/** RenderingState which uses the vertex attrib arrays from previous frames for current rendering. It makes a GPU parallel
	 * stream compaction in order to eliminate unwanted vertices and appends new vertices at the end of the vertex attrib
	 * arrays.
	 * USAGE: as usual, setPainter() should be called for setup. However, calls for handleNodeRendering() will set data to be
	 * appended to the vertex attrib arrays after compaction instead of overwriting all content from previous frames. In
	 * addition, setCompactionArray() should be called to set the compaction flags, indicating vertices that will be erased or
	 * maintained in compaction. After that, render() can be called to perform the compaction and to append the new data on the
	 * vertices attrib arrays. */
	template< typename Vec3 >
	class CompactionRenderingState
	: RenderingState< Vec3 >
	{
	public:
		CompactionRenderingState( const Attributes& attribs );
		~CompactionRenderingState();
		
		/** Sets the compaction array. Each position of this array has a boolean indicating whether the vertex at that position
		 * should stay in the vertex attrib arrays ( true ) or not ( false ). */
		void setCompactionArray( const vector< bool >& flags );
		
		unsigned int render();
		
		static const int N_MAX_VERTICES = 10000000;
	
	private:
		QOpenGLShaderProgram* m_compactionProgram;
		
		static const int N_BUFFERS = 3;
		
		/** Vertex attrib buffers. */
		QGLBuffer* m_buffers[ N_BUFFERS ];
		
		/** Transient vector that indicates which vertices will be deleted ( false ) in compaction and which will be
		 * maintained ( true ). */
		vector< bool >* m_compactionFlags;
		
		static const int BYTES_PER_VERTEX = 12;
		static const int MAX_BYTES = BYTES_PER_VERTEX * N_MAX_VERTICES;
	}
	
	template< typename Vec3 >
	CompactionRenderingState< Vec3 >::CompactionRenderingState( const Attributes& attribs )
	: RenderingState< Vec3 >( attribs )
	{
		for( int i = 0; i < N_BUFFERS; ++i)
		{
			QGLBuffer* buffer = new QGLBuffer( QGLBuffer::VertexBuffer );
			buffer->create();
			buffer->setUsagePattern( QGLBuffer::StaticDraw );
			buffer->allocate( MAX_BYTES );
			
			m_buffers[ i ] = buffer;
		}
		
		m_compactionProgram = new QOpenGLShaderProgram();
		m_compactionProgram->addShaderFromSourceFile( QOpenGLShader::Compute, "shaders/PrefixSum.comp" );
		cout << m_compactionProgram->log() << endl;
	}
	
	template< typename Vec3 >
	CompactionRenderingState< Vec3 >::~CompactionRenderingState()
	{
		for( int i = 0; i < N_BUFFERS; ++i)
		{
			QGLBuffer* buffer = m_buffers[ i ];
			buffer.destroy();
			delete buffer;
		}
	}
	
	template< typename Vec3 >
	void CompactionRenderingState< Vec3 >::setCompactionArray( const vector< bool >* flags )
	{
		m_compationFlags = flags;
	}
	
	template< typename Vec3 >
	unsigned int CompactionRenderingState< Vec3 >::render()
	{
		
	}
}

#endif