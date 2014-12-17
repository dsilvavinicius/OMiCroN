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
		m_values( values ),
		m_reduction( 0 )
		{}
		
		void ScanQGLView::initializeGL( QGLPainter *painter )
		{
			string exePath = QCoreApplication::applicationDirPath().toStdString();
			
			QOpenGLFunctions_4_3_Compatibility* openGL = context()->versionFunctions< QOpenGLFunctions_4_3_Compatibility >();
			openGL->initializeOpenGLFunctions();
			
			Scan scan( exePath + "/../shaders", 10000, openGL );
			m_reduction = scan.doScan( m_values );
			m_values = scan.getResultCPU();
		}
		
		void ScanQGLView::paintGL( QGLPainter *painter )
		{
			QApplication::quit();
		}
	}
}
