#ifndef POINT_RENDERER_WINDOW_H
#define POINT_RENDERER_WINDOW_H

#include <memory>

#include <QtGui/QGuiApplication>
#include <Qt3D/QGLCamera>
#include <Qt3D/QGLView>
#include <Qt3D/QGLShaderProgramEffect>
#include <QOpenGLFramebufferObject>

#include "Octree.h"

using namespace std;
using namespace model;

namespace ui
{
	class PointRendererWindow : public QGLView
	{
		Q_OBJECT
	public:
		PointRendererWindow( const QSurfaceFormat &format, QWindow *parent = 0 );
		~PointRendererWindow();

	protected:
		void paintGL( QGLPainter *painter );
		void initializeGL( QGLPainter *painter );
		void mouseMoveEvent( QMouseEvent* ev );
		void mousePressEvent( QMouseEvent* ev );
		void wheelEvent( QWheelEvent * ev );
		
	private:
		void drawText( QGLPainter *painter, const QRect& posn, const QString& str );
		
		ShallowOctreePtr<float, vec3> m_octree;
		QPoint m_lastMousePos;
	};

	using PointRendererWindowPtr = shared_ptr<PointRendererWindow>;
}

#endif // POINT_RENDERER_WINDOW_H
