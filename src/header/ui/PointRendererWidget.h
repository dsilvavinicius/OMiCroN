#ifndef POINT_RENDERER_WIDGET_H
#define POINT_RENDERER_WIDGET_H

//#include <GL/glew.h>
//#include <phongshader.hpp>
//#include <imgSpacePBR.hpp>
#include <utils/qttrackballwidget.hpp>
#include <point_model.hpp>
#include "TucanoRenderingState.h"
#include <IndexedOctree.h>
#include <QApplication>

using namespace std;
using namespace model;

class PointRendererWidget
: public Tucano::QtTrackballWidget
{
	Q_OBJECT
	
	using MortonCode = model::ShallowMortonCode;
	using Point = model::ExtendedPoint< float, vec3 >;
	using PointReader = ExtendedPointReader;
	using Octree = model::ShallowIndexedOctree< float, vec3, Point >;
	//using Octree = model::ShallowRandomSampleOctree< float, vec3, Point >;
	using RenderingState = model::TucanoRenderingState< vec3, float >;
	
public:
	explicit PointRendererWidget( QWidget *parent );
	~PointRendererWidget();
	
	void initialize( const unsigned int& frameRate );

	/**
	* Repaints screen buffer.
	*/
	virtual void paintGL();

	/**
	* @brief Overload resize callback
	*/
	virtual void resizeGL( int width, int height );

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
	
	/** Time when a frame is finished. Used to measure performance only. In ms. */
	clock_t m_endOfFrameTime;
	
	/** Point attributes. */
	Attributes m_attribs;
};

#endif // PointRendererWidget
