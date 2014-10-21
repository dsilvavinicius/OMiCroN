#include "PointRendererWindow.h"
#include <QOpenGLFramebufferObjectFormat>
#include <QGLBuilder>
#include <QGeometryData>
#include <QGLTexture2D>
#include <QMouseEvent>
#include <QDir>
#include "PlyPointReader.h"

using namespace util;

namespace ui
{
	PointRendererWindow::PointRendererWindow(const QSurfaceFormat &format, QWindow *parent)
		: QGLView(format, parent)
		/*, m_program(0)*/
		, m_frame(0)
	{
		PointVector<float, vec3> points = PlyPointReader::read< float, vec3 >(
			"../../src/data/tempietto_dense.ply", PlyPointReader::SINGLE);
		
		m_octree = make_shared< ShallowOctree< float, vec3 > >(1);
		m_octree->build(points);
	}
	
	PointRendererWindow::~PointRendererWindow() {}

	void PointRendererWindow::initializeGL(QGLPainter *painter)
	{	
		QGLCamera* cam = camera();
		cam->setProjectionType( QGLCamera::Perspective );
		cam->setFieldOfView( 60.0f );
		cam->setNearPlane( 0.1f );
		cam->setFarPlane( 10000.0f );
		
		painter->setCamera( cam );
	}

	void PointRendererWindow::paintGL(QGLPainter *painter)
	{
		//cout << "STARTING PAINTING!" << endl;
		//m_octree->drawBoundaries(painter, false);
		
		m_octree->traverse(painter);
		QString str();
		
		int textBoxWidth = width() * 0.3;
		int textBoxHeight = height() * 0.7;
		int margin = 10;
		drawText( painter, QRect( width() - textBoxWidth - margin, margin, textBoxWidth, textBoxHeight),
				  "Debug string: I don't know anything...\nFPS: XXX\nOctree:XXX!" );
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
			if (buttons & Qt::MiddleButton)
			{
				QQuaternion rotation = cam->roll(-(float)deltaPos.y() * 0.1f);
				cam->rotateEye(rotation);
			}
			if (buttons & Qt::RightButton)
			{
				QVector3D translation = cam->translation((float)deltaPos.x() * 0.01f,
														   (float)deltaPos.y() * 0.01f, 0);
				cam->setEye(cam->eye() + translation);
				cam->setCenter(cam->center() + translation);
			}
			
			m_lastMousePos = currentPos;
		}
	}
	
	void PointRendererWindow::wheelEvent(QWheelEvent * ev)
	{
		QGLCamera* cam = camera();
		QVector3D translation = cam->translation(0.f, 0.f, (float)ev->angleDelta().y() * 0.01f);
		cam->setEye(cam->eye() + translation);
		cam->setCenter(cam->center() + translation);
	}
	
	void PointRendererWindow::mousePressEvent(QMouseEvent* ev)
	{
		m_lastMousePos = ev->globalPos();
	}
	
	// Draw text centered on the bottom of the "posn" rectangle.
	void PointRendererWindow::drawText( QGLPainter *painter, const QRect& posn, const QString& str )
	{
		cout << "Window size: " << size() << endl << endl;
		cout << "Final debug rect: " << endl << posn << endl;
		
		painter->modelViewMatrix().push();
		painter->projectionMatrix().push();
		
		QMatrix4x4 projm;
		projm.ortho( QRect( 0, 0, width(), height() ) );
		painter->projectionMatrix() = projm;
		painter->modelViewMatrix().setToIdentity();
		
		QFont f( "Helvetica", 10 );
		QFontMetrics metrics( f );
		QRect rect = metrics.boundingRect( QRect( 0, 0, posn.width(), posn.height() ),
										   Qt::AlignLeft | Qt::TextWordWrap , str );
		cout << "Bonding rect: " << endl << rect << endl;

		QImage image( rect.size(), QImage::Format_ARGB32 );
		image.fill( 0 );
		QPainter p2( &image );
		p2.setFont( f );
		p2.setPen( Qt::white );
		p2.drawText( rect, Qt::AlignLeft | Qt::TextWordWrap, str );
		p2.end();

		QGLTexture2D texture;
		texture.setImage( image );

		int x = posn.x() + posn.width() - rect.width();
		int y = posn.y() + rect.y();

		QVector2DArray vertices;
		vertices.append( x				 , y );
		vertices.append( x				 , y + rect.height() );
		vertices.append( x + rect.width(), y + rect.height() );
		vertices.append( x + rect.width(), y );
		
		QVector2DArray texCoord;
		texCoord.append( 0.0f, 1.0f );
		texCoord.append( 0.0f, 0.0f );
		texCoord.append( 1.0f, 0.0f );
		texCoord.append( 1.0f, 1.0f );

		painter->clearAttributes();
		painter->setStandardEffect( QGL::FlatReplaceTexture2D );
		texture.bind();
		painter->setVertexAttribute( QGL::Position, vertices );
		painter->setVertexAttribute( QGL::TextureCoord0, texCoord );
		painter->draw( QGL::TriangleFan, 4 );
		painter->setStandardEffect( QGL::FlatColor );
		texture.release();
		
		painter->projectionMatrix().pop();
		painter->modelViewMatrix().pop();
	}
}