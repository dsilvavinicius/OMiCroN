#include "Scan.h"

#include <iostream>
#include <QOpenGLFunctions_4_3_Core>

using namespace std;

namespace model
{
	Scan::Scan( const string& shaderFolder, const vector< unsigned int >& values )
	{
		for( int i = 0; i < N_BUFFER_TYPES; ++i )
		{
			auto buffer = new QOpenGLBuffer( QOpenGLBuffer::VertexBuffer );
			buffer->create();
			m_buffers[ i ] = buffer;
		}
		
		m_buffers[ ORIGINAL ]->setUsagePattern( QOpenGLBuffer::StreamDraw );
		m_buffers[ SCAN_RESULT ]->setUsagePattern( QOpenGLBuffer::StaticDraw );
		m_buffers[ GLOBAL_PREFIXES ]->setUsagePattern( QOpenGLBuffer::StaticDraw );
		
		m_nElements = values.size();
		m_nBlocks = ceil( ( float ) m_nElements / BLOCK_SIZE );
		
		m_buffers[ ORIGINAL ]->allocate( (void *) &values , m_nElements * 4 );
		m_buffers[ SCAN_RESULT ]->allocate( m_nElements * 4 );
		m_buffers[ GLOBAL_PREFIXES ]->allocate( m_nBlocks );
		
		for( int i = 0; i < N_PROGRAM_TYPES; ++i )
		{
			auto program = new QOpenGLShaderProgram();
			
			switch( i )
			{
				case PER_BLOCK_SCAN :
				{
					program->addShaderFromSourceFile( QOpenGLShader::Compute, (shaderFolder + "/PerBlockScan.comp").c_str() );
					break;
				}
				case GLOBAL_SCAN :
				{
					program->addShaderFromSourceFile( QOpenGLShader::Compute, (shaderFolder + "/GlobalScan.comp").c_str() );
					break;
				}
				case FINAL_SUM :
				{
					program->addShaderFromSourceFile( QOpenGLShader::Compute, (shaderFolder + "/Sum.comp").c_str() ); break;
				}
			}
			
			program->link();
			cout << program->log().toStdString();
			
			program->bind();
			
			switch( i )
			{
				case PER_BLOCK_SCAN :
				{
					m_buffers[ ORIGINAL ]->bind();
					program->setAttributeBuffer( "original", GL_UNSIGNED_INT , 0, 1);
					
					m_buffers[ PER_BLOCK_SCAN ]->bind();
					program->setAttributeBuffer( "perBlockScan", GL_UNSIGNED_INT , 0, 1);
					break;
				}
				case GLOBAL_SCAN :
				{
					m_buffers[ ORIGINAL ]->bind();
					program->setAttributeBuffer( "original", GL_UNSIGNED_INT , 0, 1);
					
					m_buffers[ PER_BLOCK_SCAN ]->bind();
					program->setAttributeBuffer( "perBlockScan", GL_UNSIGNED_INT , 0, 1);
					
					m_buffers[ GLOBAL_PREFIXES ]->bind();
					program->setAttributeBuffer( "globalPrefixes", GL_UNSIGNED_INT , 0, 1);
					
					break;
				}
				case FINAL_SUM :
				{
					m_buffers[ PER_BLOCK_SCAN ]->bind();
					program->setAttributeBuffer( "scan", GL_UNSIGNED_INT , 0, 1);
					
					m_buffers[ GLOBAL_PREFIXES ]->bind();
					program->setAttributeBuffer( "globalPrefixes", GL_UNSIGNED_INT , 0, 1);
					
					break;
				}
			}
			
			m_programs[ i ] = program;
		}
	}
	
	Scan::~Scan()
	{
		for( int i = 0; i < N_PROGRAM_TYPES; ++i )
		{
			QOpenGLShaderProgram* program = m_programs[ i ];
			program->release();
			program->removeAllShaders();
			delete program;
			m_programs[ i ] = nullptr;
		}
		
		for( int i = 0; i < N_BUFFER_TYPES; ++i )
		{
			QOpenGLBuffer* buffer = m_buffers[ i ];
			buffer->destroy();
			delete buffer;
			m_buffers[ i ] = nullptr;
		}
	}
	
	void Scan::doScan()
	{
		QOpenGLFunctions_4_3_Core openGLFunctions;
		m_programs[ PER_BLOCK_SCAN ]->bind();
		openGLFunctions.glDispatchCompute( m_nBlocks, 1, 1 );
		
		m_programs[ GLOBAL_SCAN ]->bind();
		openGLFunctions.glDispatchCompute( 1, 1, 1 );
		
		m_programs[ FINAL_SUM ]->bind();
		openGLFunctions.glDispatchCompute( m_nBlocks, 1, 1 );
	}
	
	shared_ptr< vector< unsigned int > > Scan::getResultCPU()
	{
		vector< unsigned int > result( m_nElements );
		m_buffers[ SCAN_RESULT ]->read( 0, ( void * ) &result, m_nElements );
		
		return make_shared< vector< uint > >( result );
	}
}