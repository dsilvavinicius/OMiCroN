#include "PointRendererWindow.h"
#include <QMouseEvent>

namespace ui
{
	PointRendererWindow::PointRendererWindow()
		: m_program(0)
		, m_frame(0)
	{
		m_camera = make_shared<Camera>();
	}

	GLuint PointRendererWindow::loadShader(GLenum type, const char *source)
	{
		GLuint shader = glCreateShader(type);
		glShaderSource(shader, 1, &source, 0);
		glCompileShader(shader);
		return shader;
	}

	void PointRendererWindow::initialize()
	{
		m_program = new QOpenGLShaderProgram(this);
		m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, PointRendererWindow::vertexShaderSource);
		m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, PointRendererWindow::fragmentShaderSource);
		m_program->link();
		m_posAttr = m_program->attributeLocation("posAttr");
		m_colAttr = m_program->attributeLocation("colAttr");
		m_matrixUniform = m_program->uniformLocation("matrix");
	}

	void PointRendererWindow::render()
	{
		const qreal retinaScale = devicePixelRatio();
		glViewport(0, 0, width() * retinaScale, height() * retinaScale);
		
		glClear(GL_COLOR_BUFFER_BIT);

		m_program->bind();

		vec3 cameraEye = m_camera->getOrigin();
		vec3 cameraCenter = cameraEye + m_camera->getForward();
		vec3 cameraUp = m_camera->getUp();
		
		QMatrix4x4 matrix;
		matrix.perspective(60.0f, 4.0f/3.0f, 0.1f, 100.0f);
		matrix.lookAt(QVector3D(cameraEye.x, cameraEye.y, cameraEye.z),
			QVector3D(cameraCenter.x, cameraCenter.y, cameraCenter.z),
			QVector3D(cameraUp.x, cameraUp.y, cameraUp.z)
		);
		matrix.translate(0, 0, -2);
		matrix.rotate(100.0f * m_frame / screen()->refreshRate(), 0, 1, 0);

		m_program->setUniformValue(m_matrixUniform, matrix);

		GLfloat vertices[] = {
			0.0f, 0.707f,
			-0.5f, -0.5f,
			0.5f, -0.5f
		};

		GLfloat colors[] = {
			1.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 1.0f
		};

		glVertexAttribPointer(m_posAttr, 2, GL_FLOAT, GL_FALSE, 0, vertices);
		glVertexAttribPointer(m_colAttr, 3, GL_FLOAT, GL_FALSE, 0, colors);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glDrawArrays(GL_TRIANGLES, 0, 3);

		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(0);

		m_program->release();

		++m_frame;
	}

	void PointRendererWindow::mouseMoveEvent(QMouseEvent * ev)
	{
		Qt::MouseButtons buttons = ev->buttons();
		
		if (buttons & (Qt::LeftButton | Qt::RightButton | Qt::MiddleButton))
		{
			QPoint currentPos = ev->globalPos();
			QPoint deltaPos = currentPos - m_lastMousePos;
			if (buttons & Qt::LeftButton)
			{
				m_camera->rotateX((float)deltaPos.y() * 0.001f);
				m_camera->rotateY((float)deltaPos.x() * 0.001f);
			}
			if (buttons & Qt::RightButton)
			{
				m_camera->panX((float)deltaPos.x() * 0.01f);
				m_camera->panY((float)deltaPos.y() * 0.01f);
			}
			if (buttons & Qt::MiddleButton)
			{
				m_camera->zoom((float)deltaPos.y() * -0.01f);
			}
			
			m_lastMousePos = currentPos;
		}
	}
	
	void PointRendererWindow::mousePressEvent(QMouseEvent* ev)
	{
		m_lastMousePos = ev->globalPos();
	}
}