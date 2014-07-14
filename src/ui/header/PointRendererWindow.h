#ifndef POINT_RENDERER_WINDOW_H
#define POINT_RENDERER_WINDOW_H

#include "ui/header/OpenGLWindow.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QMatrix4x4>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QScreen>

#include <QtCore/qmath.h>

class PointRendererWindow : public OpenGLWindow
{
public:
    PointRendererWindow();

    void initialize();
    void render();
	
	static constexpr char *vertexShaderSource =
		"attribute highp vec4 posAttr;\n"
		"attribute lowp vec4 colAttr;\n"
		"varying lowp vec4 col;\n"
		"uniform highp mat4 matrix;\n"
		"void main() {\n"
		"   col = colAttr;\n"
		"   gl_Position = matrix * posAttr;\n"
		"}\n";

	static constexpr char *fragmentShaderSource =
		"varying lowp vec4 col;\n"
		"void main() {\n"
		"   gl_FragColor = col;\n"
		"}\n";

private:
    GLuint loadShader(GLenum type, const char *source);

    GLuint m_posAttr;
    GLuint m_colAttr;
    GLuint m_matrixUniform;

    QOpenGLShaderProgram *m_program;
    int m_frame;
};

#endif // POINT_RENDERER_WINDOW_H
