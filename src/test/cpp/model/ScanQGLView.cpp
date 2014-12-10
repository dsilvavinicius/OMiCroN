#include "ScanQGLView.h"

#include "Scan.h"
#include <QApplication>
//#include <QDir>

namespace model
{
	namespace test
	{
		ScanQGLView::ScanQGLView( const vector< unsigned int >& values, const QSurfaceFormat &format, QWindow *parent )
		: QGLView( format, parent ),
		m_scanResults( make_shared< vector< unsigned int > >( values ) ) {}
		
		void ScanQGLView::initializeGL( QGLPainter *painter )
		{
			string exePath = QCoreApplication::applicationDirPath().toStdString();
			Scan scan( exePath + "/../shaders", *m_scanResults );
			scan.doScan( context() );
			m_scanResults = scan.getResultCPU();
		}
		
		void ScanQGLView::paintGL( QGLPainter *painter )
		{
			QApplication::quit();
		}
	}
}
