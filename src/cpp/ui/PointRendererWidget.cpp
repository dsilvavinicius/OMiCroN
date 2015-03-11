#include "PointRendererWidget.h"
#include <QDebug>
#include <QTimer>

PointRendererWidget::PointRendererWidget(QWidget *parent)
: Tucano::QtTrackballWidget(parent),
m_projThresh( 0.001f ),
m_renderTime( 0.f ),
active_effect( 0 ),
draw_trackball( true ),
m_octree( nullptr ),
m_renderer( nullptr )
{
	//jfpbr = 0;
	//phong = 0;    
}

PointRendererWidget::~PointRendererWidget()
{
	//delete jfpbr;
	//delete phong;
	delete m_renderer;
	delete m_octree;
	delete m_timer;
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
	openMesh( "../../src/data/real/tempietto_dense.ply" );
	//Tucano::QtTrackballWidget::openMesh("./cube.ply");

	//mesh = new PointModel();
	//mesh->loadPlyFile("./cube.ply");

	//ShaderLib::MeshImporter::loadPlyFile(mesh, filename);
	//mesh->normalizeModelMatrix();

	m_timer = new QTimer( this );
	connect( m_timer, SIGNAL( timeout() ), this, SLOT( update() ) );
	m_timer->start( 16.666f ); // Update 60 fps.
}

void PointRendererWidget::resizeGL( void )
{
	camera_trackball.setViewport( Eigen::Vector2f( ( float )this->width(), ( float )this->height() ) );
	camera_trackball.setPerspectiveMatrix( camera_trackball.getFovy(), this->width() / this->height(), 0.1f,
											100.0f );
	light_trackball.setViewport( Eigen::Vector2f( ( float )this->width(), ( float )this->height() ) );

	//jfpbr->resize( this->width(), this->height() );
	updateGL();
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
	
	m_renderer->updateFrustum();
	mesh.reset();
	
	// Render the scene.
	clock_t timing = clock();
	//OctreeStats stats = m_octree->traverse( painter, m_attribs, m_projThresh );
	FrontOctreeStats stats = m_octree->trackFront( *m_renderer, m_projThresh );
	timing = clock() - timing;
	
	m_renderTime = float( timing ) / CLOCKS_PER_SEC * 1000;

	glEnable(GL_DEPTH_TEST);
	if( draw_trackball )
	{
		camera_trackball.render();
	}
}

void PointRendererWidget::toggleEffect( int id )
{
	active_effect = id;
	updateGL();
}

void PointRendererWidget::reloadShaders( void )
{
	//jfpbr->reloadShaders();
	//phong->reloadShaders();        
	updateGL();
}

void PointRendererWidget::setJFPBRFirstMaxDistance( double value )
{
	//jfpbr->setFirstMaxDistance( ( float )value );
	updateGL();
}

void PointRendererWidget::toggleDrawTrackball( void )
{
	draw_trackball = !draw_trackball;
	updateGL();
}

void PointRendererWidget::openMesh( const string& filename )
{
	Attributes vertAttribs = COLORS_AND_NORMALS;
	
	if( m_octree )
	{
		delete m_octree;
	}
	m_octree = new ShallowFrontOctree( 1, 10 );
	m_octree->build( filename, ExtendedPointReader::SINGLE, vertAttribs );
	
	mesh.reset();
	if( m_renderer )
	{
		delete m_renderer;
	}
	// Render the scene one time, traveling from octree's root to init m_renderTime for future projection
	// threshold adaptations.
	m_renderer = new TucanoRenderingState( camera_trackball, light_trackball, mesh, vertAttribs,
											QApplication::applicationDirPath().toStdString() +
											"/shaders/tucano/" );
	clock_t timing = clock();
	m_octree->traverse( *m_renderer, m_projThresh );
	timing = clock() - timing;
	m_renderTime = float( timing ) / CLOCKS_PER_SEC * 1000;
}