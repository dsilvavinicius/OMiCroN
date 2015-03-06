#include "PointRendererWidget.h"
#include <QDebug>

namespace ui
{
	PointRendererWidget::PointRendererWidget(QWidget *parent)
	: Tucano::QtTrackballWidget(parent),
	m_projThresh( 0.001f ),
	m_renderTime( 0.f )
	{
		//jfpbr = 0;
		//phong = 0;    
		active_effect = 0;
		draw_trackball = true;
		mesh = 0;
	}

	PointRendererWidget::~PointRendererWidget()
	{
		//delete jfpbr;
		//delete phong;    
	}

	void PointRendererWidget::initialize( void )
	{
		// initialize the effects
		//jfpbr = new ImgSpacePBR(this->width(), this->height());
		//jfpbr->setShadersDir("./shaders/");
		//jfpbr->initialize();

		//phong = new Effects::Phong();
		//phong->setShadersDir("../tucano/effects/shaders/");
		//phong->initialize();

		// initialize the widget, camera and light trackball, and opens default mesh
		Tucano::QtTrackballWidget::initialize();
		//Tucano::QtTrackballWidget::openMesh("./cube.ply");

		//mesh = new PointModel();
		//mesh->loadPlyFile("./cube.ply");

		//ShaderLib::MeshImporter::loadPlyFile(mesh, filename);
		//mesh->normalizeModelMatrix();
		
		// Render the scene one time to init m_renderTime for future projection threshold adaptations.
		clock_t timing = clock();
		m_octree->traverse( painter, m_attribs, m_projThresh );
		timing = clock() - timing;
		m_renderTime = float( timing ) / CLOCKS_PER_SEC * 1000;
	
		m_timer = new QTimer( this );
		connect( m_timer, SIGNAL( timeout() ), this, SLOT( update() ) );
		m_timer->start( 16.666f ); // Update 60 fps.
	}

	void PointRendererWidget::adaptProjThresh( float desiredRenderTime )
	{
		float renderTimeDiff = m_renderTime - desiredRenderTime;
		m_projThresh += renderTimeDiff * 1.0e-6f;
		m_projThresh = std::max( m_projThresh, 1.0e-15f );
		m_projThresh = std::min( m_projThresh, 1.f );
	}

	void PointRendererWidget::paintGL (void)
	{
		makeCurrent();

		glClearColor(1.0, 1.0, 1.0, 0.0);
		glClear( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT );

		/*if (mesh)
		{
			if (active_effect == 0 && phong)
			{
				phong->render(mesh, camera_trackball, light_trackball);
			}
			if (active_effect == 1 && jfpbr)
			{
				jfpbr->render(mesh, camera_trackball, light_trackball, true);

			}
		}*/
		
		//cout << "STARTING PAINTING!" << endl;
		//m_octree->drawBoundaries(painter, true);
		
		adaptProjThresh( 66.666f ); // 15 fps.
		//adaptProjThresh( 33.333f ); // 30 fps.
		//adaptProjThresh( 100.f ); // 10 fps.
		
		// Render the scene.
		clock_t timing = clock();
		//OctreeStats stats = m_octree->traverse( painter, m_attribs, m_projThresh );
		FrontOctreeStats stats = m_octree->trackFront( painter, m_attribs, m_projThresh );
		timing = clock() - timing;
		
		m_renderTime = float( timing ) / CLOCKS_PER_SEC * 1000;

		glEnable(GL_DEPTH_TEST);
		if( draw_trackball )
		{
			camera_trackball->render();
		}
	}
}