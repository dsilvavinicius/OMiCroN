#ifndef COMPACTION_RENDERING_STATE_H
#define COMPACTION_RENDERING_STATE_H

#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QCoreApplication>
#include "Scan.h"
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
	: public RenderingState< Vec3 >
	{
	public:
		/** @param posBuffer is the buffer with vertex positions.
		 *  @param attrib0Buffer is the buffer with first vertex attribute ( normal or color).
		 *	@param attrib1Buffer is the buffer with second vertex attribute ( normal ). Can be NULL if the vertices have only first attributes. If not NULL, first attribute is assumed to be color. */
		CompactionRenderingState( QOpenGLBuffer* posBuffer, QOpenGLBuffer* attrib0Buffer, QOpenGLBuffer* attrib1Buffer,
								  QOpenGLFunctions_4_3_Compatibility* openGL, const Attributes& attribs );
		~CompactionRenderingState();
		
		/** Sets the compaction array. Each position of this array has a boolean indicating whether the vertex at that position
		 * should stay in the vertex attrib arrays ( true ) or not ( false ). */
		void setCompactionArray( const vector< bool >& flags );
		
		unsigned int render();
		
		static const int N_MAX_VERTICES = 1000000;
	
	private:
		enum BufferType
		{
			POS,
			ATTRIB0,
			ATTRIB1,
			N_BUFFERS
		};
		
		QOpenGLShaderProgram* m_compactionProgram;
		
		Scan scan;
		
		QOpenGLBuffer* m_inputBuffers[ 3 ];
		QOpenGLBuffer* m_outputBuffers[ 3 ];
		
		/** Transient vector that indicates which vertices will be deleted ( false ) in compaction and which will be
		 * maintained ( true ). */
		vector< bool >* m_compactionFlags;
		
		static const int BYTES_PER_VERTEX = 12;
		static const int MAX_BYTES = BYTES_PER_VERTEX * N_MAX_VERTICES;
	};
	
	template< typename Vec3 >
	CompactionRenderingState< Vec3 >::CompactionRenderingState( QOpenGlBuffer* posBuffer, QOpenGLBuffer* attrib0Buffer,
																QOpenGLBuffer* attrib1Buffer,
															 QOpenGLFunctions_4_3_Compatibility* openGL,
															 const Attributes& attribs )
	: RenderingState< Vec3 >( attribs ),
	scan( QCoreApplication::applicationDirPath().toStdString() + "/shaders", N_MAX_VERTICES, openGL ),
	m_inputBuffers[ POS ]( vertexBuffer ),
	m_inputBuffers[ ATTRIB0 ]( attrib0Buffer ),
	m_inputBuffers[ ATTRIB1 ]( attrib1Buffer )
	{
			
		for( int i = 0; i < N_BUFFERS; ++i)
		{
			if( m_inputBuffers != NULL )
			{
				m_outputVertexBuffer = new QOpenGLBuffer( QOpenGLBuffer::VertexBuffer );
				m_outputVertexBuffer->create();
				m_outputVertexBuffer->setUsagePattern( QGLBuffer::StaticDraw );
				m_outputVertexBuffer->bind();
				m_outputVertexBuffer->allocate( MAX_BYTES );
				m_buffers[ i ] = buffer;
			}
		}
		
		m_compactionProgram = new QOpenGLShaderProgram();
		m_compactionProgram->addShaderFromSourceFile( QOpenGLShader::Compute, "shaders/PointCompaction.comp" );
		cout << m_compactionProgram->log() << endl;
	}
	
	template< typename Vec3 >
	CompactionRenderingState< Vec3 >::~CompactionRenderingState()
	{
		for( int i = 0; i < N_BUFFERS; ++i)
		{
			QOpenGLBuffer* buffer = m_outputBuffers[ i ];
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