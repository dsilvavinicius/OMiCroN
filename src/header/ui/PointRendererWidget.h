#ifndef POINT_RENDERER_WIDGET_H
#define POINT_RENDERER_WIDGET_H

//#include <GL/glew.h>
//#include <phongshader.hpp>
//#include <imgSpacePBR.hpp>
#include <utils/qttrackballwidget.hpp>
#include <point_model.hpp>
#include "TucanoRenderingState.h"
#include <FrontOctree.h>
#include <QApplication>

using namespace std;
using namespace model;

class PointRendererWidget
: public Tucano::QtTrackballWidget
{
	Q_OBJECT
	using TucanoRenderingState = model::TucanoRenderingState< vec3, float >;
	using ShallowFrontOctree = model::ShallowFrontOctree< float, vec3, ExtendedPoint< float, vec3 >,
														  unordered_set< ShallowMortonCode >  >;
	
public:
	explicit PointRendererWidget( QWidget *parent );
	~PointRendererWidget();
	
	/**
	* @brief Initializes the shader effect
	*/
	void initialize();

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
	/**
	* @brief Toggles mean filter flag
	*/
	void toggleEffect( int id );
	
	/**
	* @brief Reload effect shaders.
	*/
	void reloadShaders( void );

	/**
	* @brief Modifies the SSAO global intensity value.
	* @param value New intensity value.
	*/
	void setJFPBRFirstMaxDistance( double value );

	/**
	* @brief Toggle draw trackball flag.
	*/
	void toggleDrawTrackball( void );

	virtual void openMesh( const string& filename );

signals:
	/** Signals that the per-frame debug info is generated and should be presented. */
	void debugInfoDefined( const QString& debugInfo );

private:
	
	void adaptProjThresh( float desiredRenderTime );
	
	/// Jump-Flooding Point Based Renderer
	//ImgSpacePBR *jfpbr;

	/// Simple Phong Shader
	//Effects::Phong *phong;

	//PointModel *mesh;

	/// ID of active effect
	int active_effect;

	/// Flag to draw or not trackball
	bool draw_trackball;

	PointModel mesh;
	TucanoRenderingState* m_renderer;
	ShallowFrontOctree* m_octree;
	
	QTimer *m_timer;
	
	// Adaptive projection threshold related data.
	
	/** Current projection threshold used in octree traversal. */
	float m_projThresh;
	/** Current render time used to adapt the projection threshold. */
	float m_renderTime;
	
	/** Point attributes. */
	Attributes m_attribs;
};

#endif // PointRendererWidget
