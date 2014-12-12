#ifndef DUMMY_QGLVIEW_H
#define DUMMY_QGLVIEW_H

#include <vector>
#include <memory>
#include <QGLView>

using namespace std;

namespace model
{
	namespace test
	{
		class ScanQGLView
		: public QGLView
		{
		public:
			ScanQGLView( const vector< unsigned int >& values, const QSurfaceFormat &format, QWindow *parent = 0 );
			
			vector< unsigned int > m_values;
		
		protected:
			void initializeGL( QGLPainter * painter );
			void paintGL( QGLPainter * painter );
		private:
			
		};
	}
}

#endif