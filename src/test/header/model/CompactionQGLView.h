#ifndef COMPACTION_QGLVIEW_H
#define COMPACTION_QGLVIEW_H

#include <vector>
#include <memory>
#include <QGLView>

using namespace std;

namespace model
{
	namespace test
	{
		class CompactionQGLView
		: public QGLView
		{
		public:
			CompactionQGLView( const vector< unsigned int >& flags, const vector< vec3 >& pos, const vector< vec3 > attrib0,
							   const QSurfaceFormat &format, QWindow *parent = 0 );
		
		protected:
			void initializeGL( QGLPainter * painter );
			void paintGL( QGLPainter * painter );
		private:
			
		};
	}
}

#endif