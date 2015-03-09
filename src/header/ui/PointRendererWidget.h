#ifndef POINT_RENDERER_WIDGET_H
#define POINT_RENDERER_WIDGET_H

//#include <GL/glew.h>
//#include <phongshader.hpp>
//#include <imgSpacePBR.hpp>
#include <utils/qttrackballwidget.hpp>
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
	using ShallowFrontOctreePtr = model::ShallowFrontOctreePtr< float, vec3, Point< float, vec3 >,
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
	virtual void resizeGL( void )
	{
		camera_trackball->setViewport( Eigen::Vector2f( ( float )this->width(), ( float )this->height() ) );
		camera_trackball->setPerspectiveMatrix( camera_trackball->getFovy(), this->width() / this->height(), 0.1f,
												100.0f );
		light_trackball->setViewport( Eigen::Vector2f( ( float )this->width(), ( float )this->height() ) );

		//jfpbr->resize( this->width(), this->height() );
		updateGL();
	}

signals:
public slots:
	/**
	* @brief Toggles mean filter flag
	*/
	void toggleEffect( int id )
	{
		active_effect = id;
		updateGL();
	}
	

	/**
	* @brief Reload effect shaders.
	*/
	void reloadShaders( void )
	{
		//jfpbr->reloadShaders();
		//phong->reloadShaders();        
		updateGL();
	}

	/**
	* @brief Modifies the SSAO global intensity value.
	* @param value New intensity value.
	*/
	void setJFPBRFirstMaxDistance( double value )
	{
		//jfpbr->setFirstMaxDistance( ( float )value );
		updateGL();
	}

	/**
	* @brief Toggle draw trackball flag.
	*/
	void toggleDrawTrackball( void )
	{
		draw_trackball = !draw_trackball;
		updateGL();
	}

	virtual void openMesh( string filename )
	{
		mesh.reset();
		
		Attributes vertAttribs = COLORS_AND_NORMALS;
		
		m_octree = make_shared< ShallowFrontOctree< float, vec3, Point< float, vec3 >,
								unordered_set< ShallowMortonCode > > >( 1, 10 );
		m_octree->build( filename, SimplePointReader::SINGLE, vertAttribs );
		
		if( m_renderer )
		{
			delete m_renderer;
		}
		m_renderer = new TucanoRenderingState( camera_trackball, light_trackball, &mesh, vertAttribs,
												QApplication::applicationDirPath().toStdString() + "/shaders/tucano/" );
	}

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

	TucanoRenderingState* m_renderer;
	ShallowFrontOctreePtr m_octree;
	
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
