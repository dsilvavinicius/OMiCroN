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
		/** Draws debug text in this window. */
		void drawText( QGLPainter *painter, const QRect& posn, const QString& str );
		
		/** Adapts the octree traversal projection threshold by checking the derivative of the render time function r( p )
		 * dependent of the projection threshold p. If the rendering time differential is less than a predefined
		 * epsilon, no adaptation is done at all.
		 * @returns true if the adaptation was done, false otherwise. */
		bool adaptProjThresh( float desiredRenderTime );
		
		ShallowOctreePtr<float, vec3> m_octree;
		QPoint m_lastMousePos;
		
		// Adaptive projection threshold related data.
		
		/** Current projection threshold used in octree traversal. */
		float m_projThresh;
		/** Previous frame projection threshold. Used to calculate a differential in order to adapt the projection threshold
		 * to achieve a desired render time. */
		float m_prevProjThresh;
		/** Current render time. */
		float m_renderTime;
		/** Previous frame render time. Used to calculate a differential in order to adapt the projection threshold
		 * to achieve a desired render time. */
		float m_prevRenderTime;
	};

	using PointRendererWindowPtr = shared_ptr<PointRendererWindow>;
}

#endif // POINT_RENDERER_WINDOW_H
