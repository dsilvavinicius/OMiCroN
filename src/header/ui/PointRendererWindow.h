#ifndef POINT_RENDERER_WINDOW_H
#define POINT_RENDERER_WINDOW_H

#include <memory>

#include <QtGui/QGuiApplication>
#include <Qt3D/QGLCamera>
#include <Qt3D/QGLView>
#include <Qt3D/QGLShaderProgramEffect>
#include <QOpenGLFramebufferObject>
#include <QTimer>

//#include "Octree.h"
#include "RandomSampleOctree.h"

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
		
		/** Adapts the octree traversal projection threshold. The algorithm just increments or decrements the threshold
		 * based on the difference of the last frame rendering time and the desired rendering time. Care is taken to ensure
		 * that the threshold doesn't pass predefined upper and lower bounds.
		 * @returns true if the adaptation was done, false otherwise. */
		void adaptProjThresh( float desiredRenderTime );
		
		//ShallowOctreePtr< float, vec3, Point< float, vec3 > > m_octree;
		//MediumOctreePtr< float, vec3, Point< float, vec3 > > m_octree;
		//ShallowRandomSampleOctreePtr< float, vec3, Point< float, vec3 > > m_octree;
		MediumRandomSampleOctreePtr< float, vec3, Point< float, vec3 > > m_octree;
		QPoint m_lastMousePos;
		
		QTimer *m_timer;
		
		// Adaptive projection threshold related data.
		
		/** Current projection threshold used in octree traversal. */
		float m_projThresh;
		/** Current render time used to adapt the projection threshold. */
		float m_renderTime;
		
		bool m_normalsEnabled;
	};

	using PointRendererWindowPtr = shared_ptr<PointRendererWindow>;
}

#endif // POINT_RENDERER_WINDOW_H
