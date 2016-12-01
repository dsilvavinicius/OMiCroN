#ifndef POINT_RENDERER_WIDGET_H
#define POINT_RENDERER_WIDGET_H

#include <utils/qtfreecamerawidget.hpp>
#include <point_model.hpp>
#include "FastParallelOctree.h"
#include "renderers/StreamingRenderer.h"
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
	using Octree = FastParallelOctree< MortonCode >;
	using NodeLoader = typename Octree::NodeLoader;
	using Renderer = SplatRenderer;

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
	
	/** Adapts the rendering threshold for the current frame based on last frame performance. */
	void adaptRenderingThresh( const float renderTime );

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
	
	/** Desired render time. It is defined as the time interval between calls of paintGL. Used to adapt the projection
	 * threshold. In ms. */
	float m_desiredRenderTime;
	
	/** Rendering time tolerance used to verify if projection threshold adaptation is needed. In ms. */
	float m_renderingTimeTolerance;
	
	/** Time when a frame is started. Used to measure performance and adapt the projection threshold. */
	chrono::system_clock::time_point m_beginOfFrameTime;
};

#endif // PointRendererWidget
