#include "PointRendererWindow.h"
#include <QMouseEvent>

namespace ui
{
	PointRendererWindow::PointRendererWindow(const QSurfaceFormat &format, QWindow *parent)
		: QGLView(format, parent)
		, m_program(0)
		, m_frame(0)
	{}
	
	PointRendererWindow::~PointRendererWindow()
	{
		delete m_program;
	}

	void PointRendererWindow::initializeGL(QGLPainter *painter)
	{
		m_program = new QGLShaderProgramEffect();
		m_program->setVertexShader(PointRendererWindow::vertexShaderSource);
		m_program->setFragmentShader(PointRendererWindow::fragmentShaderSource);
		
		painter->setUserEffect(m_program);
		
		QGLCamera* cam = camera();
		cam->setProjectionType(QGLCamera::Perspective);
		cam->setFieldOfView(60.0f);
		cam->setNearPlane(0.1f);
		cam->setFarPlane(100.0f);
		
		painter->setCamera(cam);
	}

	void PointRendererWindow::paintGL(QGLPainter *painter)
	{
		painter->clearAttributes();

		const QVector2D vertices[] = {
			QVector2D(0.0f, 0.707f),
			QVector2D(-0.5f, -0.5f),
			QVector2D(0.5f, -0.5f)
		};

		const QVector3D colors[] = {
			QVector3D(1.0f, 0.0f, 0.0f),
			QVector3D(0.0f, 1.0f, 0.0f),
			QVector3D(0.0f, 0.0f, 1.0f)
		};
		
		QArray< QVector2D > vertexArray(vertices, 3);
		QArray< QVector3D > colorArray(colors, 3);

		painter->setVertexAttribute(QGL::Position, vertexArray);
		painter->setVertexAttribute(QGL::Color, colorArray);

		painter->draw(QGL::Triangles, 3);

		++m_frame;
	}

	void PointRendererWindow::mouseMoveEvent(QMouseEvent * ev)
	{
		QGLCamera* cam = camera();
		Qt::MouseButtons buttons = ev->buttons();
		
		if (buttons & (Qt::LeftButton | Qt::RightButton | Qt::MiddleButton))
		{
			QPoint currentPos = ev->globalPos();
			QPoint deltaPos = currentPos - m_lastMousePos;
			if (buttons & Qt::LeftButton)
			{
				QQuaternion rotation = 	cam->pan(-(float)deltaPos.x() * 0.1f) *
										cam->tilt(-(float)deltaPos.y() * 0.1f);
				cam->rotateEye(rotation);
			}
			if (buttons & Qt::RightButton)
			{
				QVector3D translation = cam->translation((float)deltaPos.x() * 0.01f,
														   (float)deltaPos.y() * 0.01f, 0);
				cam->setEye(cam->eye() + translation);
				cam->setCenter(cam->center() + translation);
			}
			if (buttons & Qt::MiddleButton)
			{
				QVector3D translation = cam->translation(0.f, 0.f, -(float)deltaPos.y() * 0.1f);
				cam->setEye(cam->eye() + translation);
				cam->setCenter(cam->center() + translation);
			}
			
			m_lastMousePos = currentPos;
		}
	}
	
	void PointRendererWindow::mousePressEvent(QMouseEvent* ev)
	{
		m_lastMousePos = ev->globalPos();
	}
}