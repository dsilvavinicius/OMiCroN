#include "Scan.h"

#include <iostream>
#include <sstream>
#include <cassert>
#include <QtOpenGLExtensions/QOpenGLExtensions>

#include "Stream.h"

using namespace std;

namespace model
{
	Scan::Scan( const string& shaderFolder, unsigned int nMaxElements, QOpenGLFunctions_4_3_Compatibility* openGL )
	: m_openGL( openGL ),
	m_nMaxElements( nMaxElements )
	{
		m_openGL->glGenBuffers( N_BUFFER_TYPES, &m_buffers[ 0 ] );
		
		m_openGL->glBindBuffer( GL_SHADER_STORAGE_BUFFER, m_buffers[ ORIGINAL ] );
		m_openGL->glBufferData( GL_SHADER_STORAGE_BUFFER, sizeof( unsigned int ) * nMaxElements, NULL, GL_STREAM_COPY );
		m_openGL->glBindBufferBase( GL_SHADER_STORAGE_BUFFER, ORIGINAL, m_buffers[ ORIGINAL ] );
		
		m_openGL->glBindBuffer( GL_SHADER_STORAGE_BUFFER, m_buffers[ SCAN_RESULT ] );
		m_openGL->glBufferData( GL_SHADER_STORAGE_BUFFER, sizeof( unsigned int ) * nMaxElements, NULL, GL_STREAM_COPY );
		m_openGL->glBindBufferBase( GL_SHADER_STORAGE_BUFFER, SCAN_RESULT, m_buffers[ SCAN_RESULT ] );
		
		unsigned int nMaxBlocks = ( unsigned int ) ceil( ( float ) nMaxElements / BLOCK_SIZE );
		m_openGL->glBindBuffer( GL_SHADER_STORAGE_BUFFER, m_buffers[ GLOBAL_PREFIXES ] );
		m_openGL->glBufferData( GL_SHADER_STORAGE_BUFFER, sizeof( unsigned int ) * nMaxBlocks, NULL, GL_STREAM_COPY );
		m_openGL->glBindBufferBase( GL_SHADER_STORAGE_BUFFER, GLOBAL_PREFIXES, m_buffers[ GLOBAL_PREFIXES ] );
		
		m_openGL->glBindBuffer( GL_SHADER_STORAGE_BUFFER, m_buffers[ N_ELEMENTS ] );
		m_openGL->glBufferData( GL_SHADER_STORAGE_BUFFER, sizeof( unsigned int ), NULL, GL_STREAM_COPY );
		m_openGL->glBindBufferBase( GL_SHADER_STORAGE_BUFFER, N_ELEMENTS, m_buffers[ N_ELEMENTS ] );
		
		m_openGL->glBindBuffer( GL_SHADER_STORAGE_BUFFER, m_buffers[ REDUCTION ] );
		m_openGL->glBufferData( GL_SHADER_STORAGE_BUFFER, sizeof( unsigned int ), NULL, GL_STREAM_COPY );
		m_openGL->glBindBufferBase( GL_SHADER_STORAGE_BUFFER, REDUCTION, m_buffers[ REDUCTION ] );
		
		for( int i = 0; i < N_PROGRAM_TYPES; ++i )
		{
			auto program = new QOpenGLShaderProgram();
			
			switch( i )
			{
				case PER_BLOCK_SCAN :
				{
					program->addShaderFromSourceFile( QOpenGLShader::Compute, ( shaderFolder + "/PerBlockScan.comp" ).c_str() );
					break;
				}
				case GLOBAL_SCAN :
				{
					program->addShaderFromSourceFile( QOpenGLShader::Compute, ( shaderFolder + "/GlobalScan.comp" ).c_str() );
					break;
				}
				case FINAL_SUM :
				{
					program->addShaderFromSourceFile( QOpenGLShader::Compute, ( shaderFolder + "/Sum.comp" ).c_str() );
					break;
				}
			}
			program->link();
			cout << "Linked correctly? " << program->isLinked() << endl << "Log:" << program->log().toStdString() <<endl;
			
			program->bind();
			
			m_programs[ i ] = program;
		}
	}
	
	Scan::~Scan()
	{
		for( int i = 0; i < N_PROGRAM_TYPES; ++i )
		{
			QOpenGLShaderProgram* program = m_programs[ i ];
			program->removeAllShaders();
			program->release();
			delete program;
			m_programs[ i ] = nullptr;
		}
		
		for( int i = 0; i < N_BUFFER_TYPES; ++i )
		{
			m_openGL->glInvalidateBufferData( m_buffers[ i ] );
		}
	}
	
	unsigned int Scan::doScan( const vector< unsigned int >& values )
	{
		m_nElements = values.size();
		m_nBlocks = ( unsigned int ) ceil( ( float ) m_nElements / BLOCK_SIZE );
		
		m_openGL->glBindBuffer( GL_SHADER_STORAGE_BUFFER, m_buffers[ ORIGINAL ] );
		m_openGL->glBufferSubData( GL_SHADER_STORAGE_BUFFER, 0, sizeof( unsigned int ) * m_nElements, (void *) &values[ 0 ] );
		
		m_openGL->glBindBuffer( GL_SHADER_STORAGE_BUFFER, m_buffers[ N_ELEMENTS ] );
		m_openGL->glBufferSubData( GL_SHADER_STORAGE_BUFFER, 0, sizeof( unsigned int ), (void *) &m_nElements );
		
		QOpenGLShaderProgram* program = m_programs[ PER_BLOCK_SCAN ];
		program->bind();
		program->enableAttributeArray( "original" );
		program->enableAttributeArray( "perBlockScan" );
		
		m_openGL->glDispatchCompute( m_nBlocks, 1, 1 );
		
		program->disableAttributeArray( "original" );
		program->disableAttributeArray( "perBlockScan" );
		
		program = m_programs[ GLOBAL_SCAN ];
		program->bind();
		program->enableAttributeArray( "original" );
		program->enableAttributeArray( "perBlockScan" );
		program->enableAttributeArray( "globalPrefixes" );
		
		m_openGL->glMemoryBarrier( GL_BUFFER_UPDATE_BARRIER_BIT );
		m_openGL->glDispatchCompute( 1, 1, 1 );
		
		program->disableAttributeArray( "original" );
		program->disableAttributeArray( "perBlockScan" );
		program->disableAttributeArray( "globalPrefixes" );
		
		program = m_programs[ FINAL_SUM ];
		program->bind();
		program->enableAttributeArray( "original" );
		program->enableAttributeArray( "scan" );
		program->enableAttributeArray( "globalPrefixes" );
		program->enableAttributeArray( "nElements" );
		program->enableAttributeArray( "reduction" );
		
		m_openGL->glMemoryBarrier( GL_BUFFER_UPDATE_BARRIER_BIT );
		m_openGL->glDispatchCompute( m_nBlocks, 1, 1 );
		
		program->disableAttributeArray( "original" );
		program->disableAttributeArray( "scan" );
		program->disableAttributeArray( "globalPrefixes" );
		program->disableAttributeArray( "nElements" );
		program->disableAttributeArray( "reduction" );
		
		unsigned int reduction;
		m_openGL->glBindBuffer( GL_SHADER_STORAGE_BUFFER, m_buffers[ REDUCTION ] );
		m_openGL->glGetBufferSubData( GL_SHADER_STORAGE_BUFFER, 0, sizeof( unsigned int ), ( void * ) &reduction );
		
		return reduction;
	}
	
	vector< unsigned int > Scan::getResultCPU()
	{
		m_openGL->glMemoryBarrier( GL_BUFFER_UPDATE_BARRIER_BIT );
		
		unsigned int* result = ( unsigned int* ) malloc( sizeof( unsigned int ) * m_nElements );
		m_openGL->glBindBuffer( GL_SHADER_STORAGE_BUFFER, m_buffers[ SCAN_RESULT ] );
		m_openGL->glGetBufferSubData( GL_SHADER_STORAGE_BUFFER, 0, sizeof( unsigned int ) * m_nElements, ( void * ) result );
		
		vector< unsigned int > vectorResult( m_nElements );
		std::copy( result, result + m_nElements, vectorResult.begin() );
		
		free( result );
		
		return vectorResult;
	}
	
	void Scan::dumpBuffer( const BufferType& bufferType, ostream& out )
	{
		m_openGL->glMemoryBarrier( GL_BUFFER_UPDATE_BARRIER_BIT );
		
		switch( bufferType )
		{			
			case ORIGINAL :
			case SCAN_RESULT :
			{
				unsigned int* result = ( unsigned int* ) malloc( sizeof( unsigned int ) * m_nElements );
				m_openGL->glBindBuffer( GL_SHADER_STORAGE_BUFFER, m_buffers[ bufferType ] );
				m_openGL->glGetBufferSubData( GL_SHADER_STORAGE_BUFFER, 0, sizeof( unsigned int ) * m_nElements,
											  ( void * ) result );
				
				out << "Dumping buffer " << bufferType << endl << "{" << endl;
				for( int i = 0; i < m_nElements; ++i )
				{
					out << result[ i ] << endl;
				}
				out << "}" << endl;
				break;
			}
			case GLOBAL_PREFIXES :
			{
				unsigned int* result = ( unsigned int* ) malloc( sizeof( unsigned int ) * m_nBlocks );
				m_openGL->glBindBuffer( GL_SHADER_STORAGE_BUFFER, m_buffers[ bufferType ] );
				m_openGL->glGetBufferSubData( GL_SHADER_STORAGE_BUFFER, 0, sizeof( unsigned int ) * m_nBlocks,
											  ( void * ) result );
				
				out << "Dumping buffer " << bufferType << endl << "{" << endl;
				for( int i = 0; i < m_nBlocks; ++i )
				{
					out << result[ i ] << endl;
				}
				out << "}" << endl;
				break;
			}
		}
	}
}