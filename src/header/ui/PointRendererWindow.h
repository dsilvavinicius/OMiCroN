#ifndef POINT_RENDERER_WINDOW_H
#define POINT_RENDERER_WINDOW_H

#include <memory>

#include <QtGui/QGuiApplication>
#include <Qt3D/QGLCamera>
#include <Qt3D/QGLView>
#include <Qt3D/QGLShaderProgramEffect>

#include "Octree.h"

using namespace std;
using namespace model;

namespace ui
{
	class PointRendererWindow : public QGLView
	{
	public:
		PointRendererWindow(const QSurfaceFormat &format, QWindow *parent = 0);
		~PointRendererWindow();
		
		/*static constexpr char *vertexShaderSource =
			"attribute highp vec4 qt_Vertex;\n"
			"attribute lowp vec4 qt_Color;\n"
			"varying lowp vec4 col;\n"
			"uniform highp mat4 qt_ModelViewProjectionMatrix;\n"
			"void main() {\n"
			"   col = qt_Color;\n"
			"   gl_Position = qt_ModelViewProjectionMatrix * qt_Vertex;\n"
			"}\n";

		static constexpr char *fragmentShaderSource =
			"varying lowp vec4 col;\n"
			"void main() {\n"
			"   gl_FragColor = col;\n"
			"}\n";*/

	protected:
		void paintGL(QGLPainter *painter);
		void initializeGL(QGLPainter *painter);
		virtual void mouseMoveEvent(QMouseEvent* ev);
		virtual void mousePressEvent(QMouseEvent* ev);
		virtual void wheelEvent(QWheelEvent * ev);
		
	private:
		//QGLShaderProgramEffect* m_program;
		int m_frame;
		ShallowOctreePtr<float, vec3> m_octree;
		
		QPoint m_lastMousePos;
	};

	using PointRendererWindowPtr = shared_ptr<PointRendererWindow>;
}

#endif // POINT_RENDERER_WINDOW_H
