#include "ScanQGLView.h"

#include "Scan.h"
#include <QApplication>
#include <algorithm>

namespace model
{
	namespace test
	{
		ScanQGLView::ScanQGLView( const QSurfaceFormat &format, QWindow *parent )
		: QGLView( format, parent )
		{}
		
		void ScanQGLView::initializeGL( QGLPainter *painter )
		{
			unsigned int values[ 3000 ];
			std::fill_n( values, 3000, 1);
			
			string exePath = QCoreApplication::applicationDirPath().toStdString();
			
			QOpenGLFunctions_4_3_Compatibility* openGL = context()->versionFunctions< QOpenGLFunctions_4_3_Compatibility >();
			openGL->initializeOpenGLFunctions();
			
			Scan scan( exePath + "/../shaders", &values[ 0 ], 3000, openGL );
			scan.doScan();
			m_scanResults = scan.getResultCPU();
		}
		
		void ScanQGLView::paintGL( QGLPainter *painter )
		{
			QApplication::quit();
		}
	}
}
