#ifndef POINT_RENDERER_WIDGET_H
#define POINT_RENDERER_WIDGET_H

//#include <GL/glew.h>
//#include <phongshader.hpp>
//#include <imgSpacePBR.hpp>
#include <utils/qtfreecamerawidget.hpp>
#include <point_model.hpp>
#include "FastParallelOctree.h"
#include "StreamingRenderer.h"
//#include "FrontOctree.h"
//#include "ParallelOctree.h"
// #include "OutOfCoreDebugOctree.h"
#include <QApplication>

using namespace std;
using namespace model;

class PointRendererWidget
: public Tucano::QtFreecameraWidget
{
	Q_OBJECT

public:
	using MortonCode = MediumMortonCode;
	
	using Point = model::Point;
// 	using PointPtr = shared_ptr< Point >;
// 	using PointVector = vector< PointPtr, ManagedAllocator< PointPtr > >;
// 	using PointReader = util::PlyPointReader< Point >;
	
// 	using OctreeNode = model::OctreeNode< PointVector >;
// 	using Hierarchy = OctreeMap< MortonCode, OctreeNode >;
// 	
// 	using OctreeParams = model::OctreeParams< MortonCode, Point, OctreeNode, Hierarchy >;
// 	using Octree = model::DefaultOutOfCoreDebugOctree< OctreeParams >;
	
	using Octree = FastParallelOctree< MortonCode, Point >;
	using NodeLoader = typename Octree::NodeLoader;
	using Renderer = model::StreamingRenderer< Point >;

	explicit PointRendererWidget( NodeLoader& loader, QWidget *parent );
	~PointRendererWidget();
	
	void initialize( const unsigned int& frameRate, const int& renderingTimeTolerance );

	/**
	* Repaints screen buffer.
	*/
	virtual void paintGL();

	/**
	* @brief Overload resize callback
	*/
	virtual void resizeGL( int width, int height ) override;

protected:
	
	virtual void keyPressEvent( QKeyEvent * event ) override;
	
	virtual void keyReleaseEvent( QKeyEvent * event ) override;
	
	virtual void mousePressEvent( QMouseEvent * event ) override;
	
	virtual void mouseMoveEvent( QMouseEvent * event ) override;
	
	virtual void mouseReleaseEvent( QMouseEvent * event ) override;
	
	/** Updates the camera using the key flags.  */
	void updateFromKeyInput();
	
signals:
public slots:
	/** @brief Toggle write output to image */
	void toggleWriteFrames( void );
	
	/** @brief Toggles mean filter flag */
	void toggleEffect( int id );
	
	/** @brief Reload effect shaders. */
	void reloadShaders( void );

	/** @brief Sets the desired frame rate hint. */
	void setFrameRate( const unsigned int& frameRate );
	
	/** @brief Modifies the SSAO global intensity value.
	* @param value New intensity value. */
	void setJFPBRFirstMaxDistance( double value );

	/** @brief Toggle draw trackball flag. */
	void toggleDrawTrackball( void );

	/** Toggle the auxiliary viewports drawing. */
	void toggleDrawAuxViewports( void );
	
	/** Toggles per-node debug info rendering. */
	void toggleNodeDebugDraw( const int& value );
	
	/** Opens a new point cloud. */
	virtual void openMesh( const string& filename );
	
	/** Sets the jump flooding effect frameskip. */
	void setJfpbrFrameskip( const int& value );
	
	/** Sets the rendering time tolerance in order to verify if projection threshold adaptation is needed. In ms. */
	void setRenderingTimeTolerance( const int& tolerance );

signals:
	/** Signals that the per-frame debug info is generated and should be presented. */
	void debugInfoDefined( const QString& debugInfo );

private:
	enum Viewport
	{
		FRONT,
		SIDE,
		TOP
	};
	
	/** Adapts the rendering threshold for the next frame. */
	void adaptRenderingThresh();

	/** Renders auxiliary viewports for debugging purposes. */
	void renderAuxViewport( const Viewport& viewport );
	
	/// Flag to draw or not trackball
	bool draw_trackball;

	/** Draws auxiliary viewports flag. */
	bool m_drawAuxViewports;
	
	PointModel mesh;
	Renderer* m_renderer;
	NodeLoader& m_loader;
	Octree* m_octree;
	
	QTimer *m_timer;
	
	/** Key press booleans. */
	QMap< int, bool > m_keys;
	
	/** Current normalized distance threshold used to control octree node rendering. */
	float m_projThresh;
	
	/** Current render time used to adapt the projection threshold. In ms. */
	float m_renderTime;
	
	/** Desired render time. Used to adapt the projection threshold. In ms. */
	float m_desiredRenderTime;
	
	/** Rendering time tolerance used to verify if projection threshold adaptation is needed. In ms. */
	float m_renderingTimeTolerance;
	
	/** Time when the last user keyboard or mouse input occurred. */
	chrono::system_clock::time_point m_inputEndTime;
	
	/** Time when a frame is finished. Used to measure performance only. In ms. */
	chrono::system_clock::time_point m_endOfFrameTime;
};

#endif // PointRendererWidget
