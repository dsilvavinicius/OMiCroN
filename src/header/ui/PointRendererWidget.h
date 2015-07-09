#ifndef POINT_RENDERER_WIDGET_H
#define POINT_RENDERER_WIDGET_H

//#include <GL/glew.h>
//#include <phongshader.hpp>
//#include <imgSpacePBR.hpp>
#include <utils/qtflycamerawidget.hpp>
#include <point_model.hpp>
#include "IndexedTucanoRenderer.h"
//#include "FrontOctree.h"
//#include "ParallelOctree.h"
#include "OutOfCoreOctree.h"
#include <QApplication>

using namespace std;
using namespace model;

class PointRendererWidget
: public Tucano::QtFlycameraWidget
{
	Q_OBJECT
	
	using MortonCode = model::ShallowMortonCode;
	using Point = model::Point;
	//using Point = model::ExtendedPoint;
	using PointReader = util::SimplePointReader;
	//using PointReader = util::ExtendedPointReader;
	//using Octree = model::ShallowIndexedOctree< Point >;
	//using Octree = model::ShallowRandomSampleOctree< Point >;
	//using Octree = model::ShallowFrontOctree;
	//using Octree = model::ShallowParallelOctree< Point >;
	using Octree = model::ShallowOutOfCoreOctree;
	//using Octree = model::MediumOutOfCoreOctree;
	//using RenderingState = model::IndexedTucanoRenderer< Point >;
	using RenderingState = model::TucanoRenderingState;
	
public:
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
	virtual void resizeGL( int width, int height );

protected:
	
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
	
	/** Adaps the projection threshold given a desired render time for a frame. */
	void adaptProjThresh( float desiredRenderTime );

	/** Render auxiliary viewports for debugging purposes. */
	void renderAuxViewport( const Viewport& viewport );
	
	/// Flag to draw or not trackball
	bool draw_trackball;

	/** Draws auxiliary viewports flag. */
	bool m_drawAuxViewports;
	
	PointModel mesh;
	RenderingState* m_renderer;
	Octree* m_octree;
	
	QTimer *m_timer;
	
	/** Current projection threshold used in octree traversal. */
	float m_projThresh;
	
	/** Current render time used to adapt the projection threshold. In ms. */
	float m_renderTime;
	
	/** Desired render time. Used to adapt the projection threshold. In ms. */
	float m_desiredRenderTime;
	
	/** Rendering time tolerance used to verify if projection threshold adaptation is needed. In ms. */
	float m_renderingTimeTolerance;
	
	/** Time when a frame is finished. Used to measure performance only. In ms. */
	clock_t m_endOfFrameTime;
	
	/** Point attributes. */
	Attributes m_attribs;
};

#endif // PointRendererWidget
