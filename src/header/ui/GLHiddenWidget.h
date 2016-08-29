#ifndef _GLHiddenWidget_h
#define _GLHiddenWidget_h

#include <GL/glew.h>
#include <QtOpenGL/QGLWidget>

class GLHiddenWidget
: public QGLWidget
{
	Q_OBJECT
public:
	GLHiddenWidget( QWidget* parent = 0 );
	~GLHiddenWidget();

protected:
	void glInit();
	void glDraw();
	void initializeGL();
	void resizeGL(int width, int height);
	void paintGL();
	void paintEvent(QPaintEvent *);
	void resizeEvent(QResizeEvent *event);

private:
};

#endif // _GLHiddenWidget_h