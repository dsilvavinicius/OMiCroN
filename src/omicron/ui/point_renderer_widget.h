#ifndef POINT_RENDERER_WIDGET_H
#define POINT_RENDERER_WIDGET_H

#include <tucano/utils/qtfreecamerawidget.hpp>
#include <tucano/utils/path.hpp>
#include "omicron/basic/morton_code.h"

#include "omicron/hierarchy/runtime_setup.h"
#include "omicron/renderer/streaming_renderer.h"
#include "omicron/memory/global_malloc.h"
#include "omicron/hierarchy/reconstruction_params.h"

#if OCTREE_CONSTRUCTION == BINARY_OCTREE_FILE
	#include "omicron/hierarchy/front_octree.h"
#elif OCTREE_CONSTRUCTION == TOP_DOWN_OCTREE
	#include "omicron/hierarchy/top_down_front_octree.h"
#else
	#include "omicron/hierarchy/fast_parallel_octree.h"
#endif

#include <QApplication>

using namespace std;
using namespace omicron;

class PointRendererWidget
: public Tucano::QtFreecameraWidget
{
	Q_OBJECT

public:
	using MortonCode = MediumMortonCode;
	
	using Point = omicron::basic::Point;
	
	#if OCTREE_CONSTRUCTION == BINARY_OCTREE_FILE
		using Octree = FrontOctree< MortonCode >;
	#elif OCTREE_CONSTRUCTION == TOP_DOWN_OCTREE
		using Octree = TopDownFrontOctree< MortonCode >;
	#else
		using Octree = FastParallelOctree< MortonCode >;
	#endif
	
	using Renderer = SplatRenderer;

	explicit PointRendererWidget( QWidget *parent );
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
	
	virtual void closeEvent( QCloseEvent * event ) override;
	
	/** Updates the camera using the key flags.  */
	void updateFromKeyInput();
	
signals:
public slots:
	/** @brief Sets the desired frame rate hint. */
	void setFrameRate( const unsigned int& frameRate );

	/** @brief Toggle draw trackball flag. */
	void toggleDrawTrackball( void );

	/** Toggle the auxiliary viewports drawing. */
	void toggleDrawAuxViewports( void );
	
	/** Toggles per-node debug info rendering. */
	void toggleNodeDebugDraw( const int& value );
	
	/** Opens a new point cloud. */
	virtual void openMesh( const string& filename );
	
	/** Sets the rendering time tolerance in order to verify if projection threshold adaptation is needed. In ms. */
	void setRenderingTimeTolerance( const int& tolerance );

	/** Loads the camera path for the model. */
	void loadCameraPath();
	
	/** Loads the camera path for the model. */
	void saveCameraPath();
	
	/** Loads the camera matrix for a screenshots. */
	void loadScreenshotCamera();
	
	/** Loads the camera matrix for a screenshot. */
	void saveScreenshotCamera();
	
	/** Saves a completed octree in a file. If the hierarchy construction is not finished yet or there is another save
	 * operation pending, it does nothing. */
	void saveOctree();
	
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
	
	/** Adapts the rendering threshold for the current frame based on last frame performance. */
	void adaptRenderingThresh( const float renderTime );

	/** Renders auxiliary viewports for debugging purposes. */
	void renderAuxViewport( const Viewport& viewport );
	
	/// Flag to draw or not trackball
	bool draw_trackball;

	/** Draws auxiliary viewports flag. */
	bool m_drawAuxViewports;
	
// 	PointModel mesh;
	Renderer* m_renderer;
	Octree* m_octree;
	
	QTimer *m_timer;
	
	/** Key press booleans. */
	QMap< int, bool > m_keys;
	
	/** Current normalized distance threshold used to control octree node rendering. */
	float m_projThresh;
	
	/** Desired render time. It is defined as the time interval between calls of paintGL. Used to adapt the projection
	 * threshold. In ms. */
	float m_desiredRenderTime;
	
	/** Rendering time tolerance used to verify if projection threshold adaptation is needed. In ms. */
	float m_renderingTimeTolerance;
	
	/** Time when a frame is started. Used to measure performance and adapt the projection threshold. */
	chrono::system_clock::time_point m_beginOfFrameTime;
	
	/** Current and average statistics of the Octree. */
	CumulusStats m_statistics;
	
	/** Camera path for making automatic videos.*/
	Tucano::Path m_cameraPath;
	
	/** Future to know when the operation of saving an octree is finished. The future result is the duration of the save operation. */
	future< int > m_octSaveFuture;
};

#endif // PointRendererWidget
