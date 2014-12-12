#include "ScanQGLView.h"

#include "Scan.h"
#include <QApplication>
#include <algorithm>

namespace model
{
	namespace test
	{
		ScanQGLView::ScanQGLView( const vector< unsigned int >& values, const QSurfaceFormat &format, QWindow *parent )
		: QGLView( format, parent ),
		m_values( values )
		{}
		
		void ScanQGLView::initializeGL( QGLPainter *painter )
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
			scan.doScan( &values[ 0 ], m_values.size() );
			m_values = scan.getResultCPU();
			
			free( values );
		}
		
		void ScanQGLView::paintGL( QGLPainter *painter )
		{
			QApplication::quit();
		}
	}
}
