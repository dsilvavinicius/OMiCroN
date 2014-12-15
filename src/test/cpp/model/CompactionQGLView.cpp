#include "CompactionQGLView.h"

#include "CompactionRenderingState.h"
#include <QApplication>
#include <algorithm>

namespace model
{
	namespace test
	{
		CompactionQGLView::CompactionQGLView( const vector< unsigned int >& flags, const vector< vec3 >& pos,
											  const vector< vec3 > attrib0, const QSurfaceFormat &format, QWindow *parent )
		: QGLView( format, parent )
		{}
		
		void CompactionQGLView::initializeGL( QGLPainter *painter )
		{
			string exePath = QCoreApplication::applicationDirPath().toStdString();
			
			QOpenGLFunctions_4_3_Compatibility* openGL = context()->versionFunctions< QOpenGLFunctions_4_3_Compatibility >();
			openGL->initializeOpenGLFunctions();
			
			unsigned int* values = ( unsigned int * ) malloc( sizeof( unsigned int ) * m_values.size() );
			for( int i = 0; i < m_values.size(); ++i )
			{
				values[ i ] = m_values[ i ];
			}
			
			Scan scan( exePath + "/../shaders", 10000, openGL );
			m_reduction = scan.doScan( &values[ 0 ], m_values.size() );
			m_values = scan.getResultCPU();
			
			free( values );
		}
		
		void CompactionQGLView::paintGL( QGLPainter *painter )
		{
			QApplication::quit();
		}
	}
}

/*CompactionRenderingState( QOpenGLBuffer* inputBuffers[ 3 ], unsigned int nElements,
								  QOpenGLFunctions_4_3_Compatibility* openGL,
								  const Attributes& attribs );*/