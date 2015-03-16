#include "CompactionQGLView.h"

#include <QApplication>
#include <algorithm>

#include "CompactionRenderingState.h"

namespace model
{
	namespace test
	{
		CompactionQGLView::CompactionQGLView( const vector< unsigned int >& flags, const vector< vec3 >& pos,
											  const vector< vec3 >& attrib0, const QSurfaceFormat &format, QWindow *parent )
		: QGLView( format, parent ),
		m_flags( flags ),
		m_compactedPos( pos ),
		m_compactedAttrib0( attrib0 )
		{}
		
		void CompactionQGLView::initializeGL( QGLPainter *painter )
		{
			QOpenGLBuffer* buffers[ 3 ];
			unsigned int bufferSize = sizeof( vec3 ) * m_compactedPos.size();
			for( int i = 0; i < CompactionRenderingState< vec3, float >::ATTRIB1; ++i )
			{
				cout << "CompactionQGLView buffer creation: " << i << endl;
				QOpenGLBuffer* buffer = new QOpenGLBuffer( QOpenGLBuffer::VertexBuffer );
				buffer->create();
				buffer->setUsagePattern( QOpenGLBuffer::StaticDraw );
				buffer->bind();
				
				buffers[ i ] = buffer;
			}
			buffers[ CompactionRenderingState< vec3, float >::ATTRIB1 ] = NULL;
			
			QOpenGLBuffer* buffer = buffers[ CompactionRenderingState< vec3, float >::POS ];
			buffer->bind();
			buffer->allocate( ( void * ) &m_compactedPos[ 0 ], bufferSize );
			
			buffer = buffers[ CompactionRenderingState< vec3, float >::ATTRIB0 ];
			buffer->bind();
			buffer->allocate( ( void * ) &m_compactedAttrib0[ 0 ], bufferSize );
			
			QOpenGLFunctions_4_3_Compatibility* openGL = context()->versionFunctions< QOpenGLFunctions_4_3_Compatibility >();
			openGL->initializeOpenGLFunctions();
			
			CompactionRenderingState< vec3, float > renderingState( buffers, m_compactedPos.size(), openGL,
																	 COLORS );
			renderingState.setPainter( painter, size() );
			renderingState.setCompactionArray( m_flags );
			renderingState.render();
			
			vector< vector< vec3 > > compacted = renderingState.getResultCPU();
			m_compactedPos = compacted[ 0 ];
			m_compactedAttrib0 = compacted[ 1 ];
		}
		
		void CompactionQGLView::paintGL( QGLPainter *painter )
		{
			QApplication::quit();
		}
	}
}