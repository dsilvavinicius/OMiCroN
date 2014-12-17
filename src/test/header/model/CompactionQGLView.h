#ifndef COMPACTION_QGLVIEW_H
#define COMPACTION_QGLVIEW_H

#include <vector>
#include <QGLView>
#include <glm/glm.hpp>

using namespace std;
using namespace glm;

namespace model
{
	namespace test
	{
		class CompactionQGLView
		: public QGLView
		{
		public:
			CompactionQGLView( const vector< unsigned int >& flags, const vector< vec3 >& pos, const vector< vec3 >& attrib0,
							   const QSurfaceFormat &format, QWindow *parent = 0 );
		
			vector< unsigned int > m_flags;
			vector< vec3 > m_compactedPos;
			vector< vec3 > m_compactedAttrib0;
			
		protected:
			void initializeGL( QGLPainter * painter );
			void paintGL( QGLPainter * painter );
		};
	}
}

#endif