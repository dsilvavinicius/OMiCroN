#ifndef COMPACTION_RENDERING_STATE_H
#define COMPACTION_RENDERING_STATE_H

#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QCoreApplication>

#include "Scan.h"
#include "RenderingState.h"
#include "Stream.h"

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
		using RenderingState = model::RenderingState< Vec3 >;
	public:
		enum BufferType
		{
			POS,
			ATTRIB0,
			ATTRIB1,
			N_BUFFER_TYPES
		};
		
		/** @param inputBuffers is the input buffers array, which will be compacted latter on. This array must have the
		 * buffers in the same order as the enum BufferType. The contents of the buffers in inputBuffers are copied to
		 * internaly managed buffers and released later to conserve memory.
		 *	@param nElements is the number of elements in inputBuffers. */
		CompactionRenderingState( QOpenGLBuffer* inputBuffers[ 3 ], const unsigned int& nElements,
								  QOpenGLFunctions_4_3_Compatibility* openGL,
								  const Attributes& attribs );
		~CompactionRenderingState();
		
		/** Sets the compaction array. Each position of this array has a boolean indicating whether the vertex at that position
		 * should stay in the vertex attrib arrays ( true ) or not ( false ). */
		void setCompactionArray( const vector< unsigned int >& flags );
		
		unsigned int render();
		
		/** Transfer the results back to main memory and them. The transfer is costly, so this method should be used
		 * judiciously. Also, this method should be called after render() so the results are available. */
		vector< vector< Vec3 > > getResultCPU();
		
		static const int N_MAX_VERTICES = 1000000;
	
	private:
		/** Compacts the point stream, given that the compaction array is already set using setCompactionArray().
		 * @returns the number of points in the stream after compaction. */
		unsigned int compact();
		
		/** Dump the buffer with given buffer type to the stream for debug reasons. Transfers data from GPU to main memory,
		 * so it is costly.*/
		void dumpBuffer( const BufferType& bufferType, bool isInput, ostream& out );
		
		QOpenGLFunctions_4_3_Compatibility* m_openGL;
		QOpenGLShaderProgram* m_compactionProgram;
		
		Scan m_scan;
		
		QOpenGLBuffer* m_inputBuffers[ 3 ];
		QOpenGLBuffer* m_outputBuffers[ 3 ];
		
		/** Transient vector that indicates which vertices will be deleted ( false ) in compaction and which will be
		 * maintained ( true ). */
		vector< unsigned int > m_compactionFlags;
		
		/** Number of elements in buffers after compaction and insertion of new nodes. */
		unsigned int m_nElements;
		
		static const int BYTES_PER_VERTEX = sizeof( Vec3 );
		static const int MAX_BYTES = BYTES_PER_VERTEX * N_MAX_VERTICES;
		static const int BLOCK_SIZE = 1024;
	};
	
	template< typename Vec3 >
	CompactionRenderingState< Vec3 >::CompactionRenderingState( QOpenGLBuffer* inputBuffers[ 3 ], const unsigned int& nElements,
																QOpenGLFunctions_4_3_Compatibility* openGL,
															 const Attributes& attribs )
	: RenderingState( attribs ),
	m_openGL( openGL ),
	m_scan( QCoreApplication::applicationDirPath().toStdString() + "/../shaders", N_MAX_VERTICES, openGL ),
	m_nElements( nElements )
	{
		for( int i = 0; i < N_BUFFER_TYPES; ++i )
		{
			if( inputBuffers[ i ] != NULL )
			{
				// Allocate input buffer.
				QOpenGLBuffer* buffer = new QOpenGLBuffer( QOpenGLBuffer::VertexBuffer );
				buffer->create();
				buffer->setUsagePattern( QOpenGLBuffer::StaticDraw );
				buffer->bind();
				buffer->allocate( MAX_BYTES );
				
				// Copies parameter buffer to created buffer.
				openGL->glBindBuffer( GL_COPY_WRITE_BUFFER, buffer->bufferId() );
				openGL->glBindBuffer( GL_COPY_READ_BUFFER, inputBuffers[ i ]->bufferId() );
				openGL->glCopyBufferSubData( GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, m_nElements * BYTES_PER_VERTEX );
				
				m_inputBuffers[ i ] = buffer;
				
				// Release parameter buffer.
				inputBuffers[ i ]->destroy();
			}
			else
			{
				m_inputBuffers[ i ] = NULL;
			}
		}
		
		for( int i = 0; i < N_BUFFER_TYPES; ++i)
		{
			if( m_inputBuffers[ i ] != NULL )
			{
				// Allocate input buffer.
				QOpenGLBuffer* buffer = new QOpenGLBuffer( QOpenGLBuffer::VertexBuffer );
				buffer->create();
				buffer->setUsagePattern( QOpenGLBuffer::StaticDraw );
				buffer->bind();
				buffer->allocate( MAX_BYTES );
				
				m_outputBuffers[ i ] = buffer;
			}
			else
			{
				m_outputBuffers[ i ] = NULL;
			}
		}
		
		m_compactionProgram = new QOpenGLShaderProgram();
		m_compactionProgram->addShaderFromSourceFile( QOpenGLShader::Compute,
													  ( QCoreApplication::applicationDirPath().toStdString() +
													  "/../shaders/PointCompaction.comp" ).c_str() );
		cout << m_compactionProgram->log().toStdString() << endl;
	}
	
	template< typename Vec3 >
	CompactionRenderingState< Vec3 >::~CompactionRenderingState()
	{
		for( int i = 0; i < N_BUFFER_TYPES; ++i)
		{
			if( m_inputBuffers[ i ] != NULL )
			{
				QOpenGLBuffer* buffer = m_outputBuffers[ i ];
				buffer->destroy();
				delete buffer;
				
				buffer = m_inputBuffers[ i ];
				buffer->destroy();
				delete buffer;
			}
		}
	}
	
	template< typename Vec3 >
	void CompactionRenderingState< Vec3 >::setCompactionArray( const vector< unsigned int >& flags )
	{
		m_compactionFlags = flags;
	}
	
	template< typename Vec3 >
	unsigned int CompactionRenderingState< Vec3 >::compact()
	{
		// Makes the compaction of the unused points.
		unsigned int nElements = m_compactionFlags.size();
		unsigned int nBlocks = ( unsigned int ) ceil( ( float ) nElements / BLOCK_SIZE );
		nElements = m_scan.doScan( m_compactionFlags );
		
		m_openGL->glBindBufferBase( GL_SHADER_STORAGE_BUFFER, Scan::N_BUFFER_TYPES + POS, m_inputBuffers[ POS ]->bufferId() );
		m_openGL->glBindBufferBase( GL_SHADER_STORAGE_BUFFER, Scan::N_BUFFER_TYPES + ATTRIB0,
									m_inputBuffers[ ATTRIB0 ]->bufferId() );
		m_openGL->glBindBufferBase( GL_SHADER_STORAGE_BUFFER, Scan::N_BUFFER_TYPES + N_BUFFER_TYPES + POS,
									m_outputBuffers[ POS ]->bufferId() );
		m_openGL->glBindBufferBase( GL_SHADER_STORAGE_BUFFER, Scan::N_BUFFER_TYPES + N_BUFFER_TYPES + ATTRIB0,
									m_outputBuffers[ ATTRIB0 ]->bufferId() );
		
		m_compactionProgram->bind();
		m_compactionProgram->enableAttributeArray( "flags" );
		m_compactionProgram->enableAttributeArray( "prefixes" );
		m_compactionProgram->enableAttributeArray( "inputVertices" );
		m_compactionProgram->enableAttributeArray( "inputAttrib0" );
		m_compactionProgram->enableAttributeArray( "outputVertices" );
		m_compactionProgram->enableAttributeArray( "outputAttrib0" );
		
		m_openGL->glDispatchCompute( nBlocks, 1, 1 );
		
		m_compactionProgram->disableAttributeArray( "flags" );
		m_compactionProgram->disableAttributeArray( "prefixes" );
		m_compactionProgram->disableAttributeArray( "inputVertices" );
		m_compactionProgram->disableAttributeArray( "inputAttrib0" );
		m_compactionProgram->disableAttributeArray( "outputVertices" );
		m_compactionProgram->disableAttributeArray( "outputAttrib0" );
		
		return nElements;
	}
	
	template< typename Vec3 >
	unsigned int CompactionRenderingState< Vec3 >::render()
	{
		m_nElements = compact();
		
		/*
		// Sends new points to GPU.
		QOpenGLBuffer* buffer = m_outputBuffers[ POS ];
		buffer->bind();
		buffer->write( nElements, ( void * ) &RenderingState::m_positions[ 0 ],
					   RenderingState::m_positions.count() * BYTES_PER_VERTEX );
		
		buffer = m_outputBuffers[ ATTRIB0 ];
		buffer->bind();
		buffer->write( nElements, ( void * ) &RenderingState::m_colors[ 0 ],
					   RenderingState::m_colors.count() * BYTES_PER_VERTEX );
		*/
		// Draws the resulting points.
		
		
		// Swaps buffers for the next frame.
		for( int i = 0; i < N_BUFFER_TYPES; ++i )
		{
			std::swap( m_inputBuffers[ i ], m_outputBuffers[ i ] );
		}
	}
	
	template< typename Vec3 >
	vector< vector< Vec3 > > CompactionRenderingState< Vec3 >::getResultCPU()
	{
		unsigned int resultSize = sizeof( Vec3 ) * m_nElements;
		Vec3* result = ( Vec3* ) malloc( resultSize );
		
		vector< vector< Vec3 > > results;
		
		for( int i = 0; i < N_BUFFER_TYPES; ++i )
		{
			if( m_inputBuffers[ i ] != NULL )
			{
				m_openGL->glBindBuffer( GL_SHADER_STORAGE_BUFFER, m_inputBuffers[ i ]->bufferId() );
				m_openGL->glGetBufferSubData( GL_SHADER_STORAGE_BUFFER, 0, resultSize, ( void * ) result );
				
				vector< Vec3 > tempVec( m_nElements );
				std::copy( result, result + m_nElements, tempVec.begin() );
				results.push_back( tempVec );
			}
		}
		
		free( result );
		
		return results;
	}
	
	template< typename Vec3 >
	void CompactionRenderingState< Vec3 >::dumpBuffer( const BufferType& bufferType, bool isInput, ostream& out )
	{
		if( m_inputBuffers[ bufferType ] != NULL )
		{
			unsigned int resultSize = sizeof( Vec3 ) * m_nElements;
			Vec3* result = ( Vec3* ) malloc( resultSize );
			
			if( isInput )
			{
				m_openGL->glBindBuffer( GL_SHADER_STORAGE_BUFFER, m_inputBuffers[ bufferType ]->bufferId() );
				m_openGL->glGetBufferSubData( GL_SHADER_STORAGE_BUFFER, 0, resultSize, ( void * ) result );
			}
			else
			{
				m_openGL->glBindBuffer( GL_SHADER_STORAGE_BUFFER, m_outputBuffers[ bufferType ]->bufferId() );
				m_openGL->glGetBufferSubData( GL_SHADER_STORAGE_BUFFER, 0, resultSize, ( void * ) result );
			}
			
			vector< Vec3 > tempVec( m_nElements );
			std::copy( result, result + m_nElements, tempVec.begin() );
			
			out << "Dumping buffer " << bufferType << endl << tempVec << endl;
			
			free( result );
		}
	}
}

#endif