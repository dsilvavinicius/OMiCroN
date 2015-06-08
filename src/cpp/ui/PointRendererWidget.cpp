#include "PointRendererWidget.h"
#include <QDebug>
#include <QTimer>

PointRendererWidget::PointRendererWidget( QWidget *parent )
: Tucano::QtFlycameraWidget( parent ),
m_projThresh( 0.001f ),
m_renderTime( 0.f ),
m_desiredRenderTime( 0.f ),
m_endOfFrameTime( clock() ),
draw_trackball( true ),
m_drawAuxViewports( false ),
m_octree( nullptr ),
m_renderer( nullptr )
{
	camera.setSpeed( 3.f );
}

PointRendererWidget::~PointRendererWidget()
{
	delete m_renderer;
	delete m_octree;
	delete m_timer;
}

void PointRendererWidget::initialize( const unsigned int& frameRate, const int& renderingTimeTolerance )
{
	Tucano::QtFlycameraWidget::initialize();
	
	setFrameRate( frameRate );
	m_renderingTimeTolerance = renderingTimeTolerance;
	
	openMesh( QApplication::applicationDirPath().toStdString() + "/data/example/staypuff.ply" );
	
	m_timer = new QTimer( this );
	connect( m_timer, SIGNAL( timeout() ), this, SLOT( update() ) );
	m_timer->start( 16.666f ); // Update 60 fps.
}

void PointRendererWidget::resizeGL( int width, int height )
{
	camera.setViewport( Eigen::Vector2f( ( float )width, ( float )height ) );
	camera.setPerspectiveMatrix( camera.getFovy(), width / height, 0.1f, 10000.0f );
	light_trackball.setViewport( Eigen::Vector2f( ( float )width, ( float )height ) );

	if( m_renderer )
	{
		m_renderer->getJumpFlooding().resize( width, height );
	}
	
	updateGL();
}


void PointRendererWidget::adaptProjThresh( float desiredRenderTime )
{
	float renderTimeDiff = m_renderTime - desiredRenderTime;
	if( abs( renderTimeDiff ) > m_renderingTimeTolerance )
	{
		m_projThresh += renderTimeDiff * 1.0e-6f;
		m_projThresh = std::max( m_projThresh, 1.0e-15f );
		m_projThresh = std::min( m_projThresh, 1.f );
	}
}

void PointRendererWidget::paintGL (void)
{
	clock_t startOfFrameTime = clock();
	clock_t totalTiming = startOfFrameTime;
	makeCurrent();

	glClearColor(1.0, 1.0, 1.0, 0.0);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	//cout << "STARTING PAINTING!" << endl;
	//m_octree->drawBoundaries(painter, true);
	
	adaptProjThresh( m_desiredRenderTime );
	
	m_renderer->clearAttribs();
	//m_renderer->clearIndices();
	m_renderer->updateFrustum();
	
	// Render the scene.
	clock_t timing = clock();
	//OctreeStats stats = m_octree->traverse( *m_renderer, m_projThresh );
	FrontOctreeStats stats = m_octree->trackFront( *m_renderer, m_projThresh );
	timing = clock() - timing;
	
	m_renderTime = float( timing ) / CLOCKS_PER_SEC * 1000;

	totalTiming = clock() - totalTiming;
	
	// Render debug data.
	stringstream debugSS;
	debugSS << "Total loop time: " << float( totalTiming ) / CLOCKS_PER_SEC * 1000 << endl << endl
			<< "Render time (traversal + rendering): " << m_renderTime << " ms" << endl << endl
			<< "Time between frames: " << float( startOfFrameTime - m_endOfFrameTime ) / CLOCKS_PER_SEC * 1000 <<
			"ms" << endl
			<< stats
			<< "Desired render time: " << m_desiredRenderTime << "ms" << endl << endl
			<< "Rendering time tolerance: " << m_renderingTimeTolerance << "ms" << endl << endl
			<< "Projection threshold: " << m_projThresh << endl << endl;
			
	//cout << debugSS.str() << endl << endl;
	
	int textBoxWidth = width() * 0.3;
	int textBoxHeight = height() * 0.7;
	int margin = 10;
	debugInfoDefined( QString( debugSS.str().c_str() ) );
	
	glEnable(GL_DEPTH_TEST);
	if( draw_trackball )
	{
		camera.renderAtCorner();
	}
	
	m_endOfFrameTime = clock();
	
	if( m_drawAuxViewports )
	{
		glEnable( GL_SCISSOR_TEST );
		renderAuxViewport( FRONT );
		renderAuxViewport( SIDE );
		renderAuxViewport( TOP );
		glDisable( GL_SCISSOR_TEST );
	}
}

void PointRendererWidget::renderAuxViewport( const Viewport& viewport )
{
	Vector2f viewportPos;
	
	switch( viewport )
	{
		case FRONT: viewportPos[ 0 ] = 0.f; viewportPos[ 1 ] = 0.f; break;
		case SIDE: viewportPos[ 0 ] = size().width() * 0.333f; viewportPos[ 1 ] = 0.f; break;
		case TOP: viewportPos[ 0 ] = size().width() * 0.666f; viewportPos[ 1 ] = 0.f; break;
	}
	
	Vector4f auxViewportSize( viewportPos[ 0 ], viewportPos[ 1 ], size().width() * 0.333f, size().height() * 0.333f );
	glScissor( auxViewportSize.x(), auxViewportSize.y(), auxViewportSize.z(), auxViewportSize.w() );
	glClear( GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT );
	
	Trackball tempCamera;
	tempCamera.setViewport( auxViewportSize );
	tempCamera.setPerspectiveMatrix( tempCamera.getFovy(), auxViewportSize.z() / auxViewportSize.w(), 0.1f, 10000.0f );
	tempCamera.resetViewMatrix();
	
	switch( viewport )
	{
		case FRONT:
		{
			 tempCamera.translate( Vector3f( 0.f, 0.f, -200.f ) );
			 break;
		}
		case SIDE:
		{
			tempCamera.rotate( Quaternionf( AngleAxisf( 0.5 * M_PI, Vector3f::UnitY() ) ) );
			tempCamera.translate( Vector3f( 200.f, 0.f, 0.f ) );
			break;
		}
		case TOP:
		{
			tempCamera.rotate( Quaternionf( AngleAxisf( 0.5 * M_PI, Vector3f::UnitX() ) ) );
			tempCamera.translate( Vector3f( 0.f, -200.f, 0.f ) );
			break;
		}
	}
	
	Phong &phong = m_renderer->getPhong();
	phong.render( mesh, tempCamera, light_trackball );
}

void PointRendererWidget::toggleWriteFrames()
{
	m_renderer->getJumpFlooding().toggleWriteFrames();	
	updateGL();
}

void PointRendererWidget::toggleEffect( int id )
{
	m_renderer->selectEffect( ( RenderingState::Effect ) id );
	updateGL();
}

void PointRendererWidget::reloadShaders( void )
{
	m_renderer->getPhong().reloadShaders();
	m_renderer->getJumpFlooding().reloadShaders();
	updateGL();
}

void PointRendererWidget::setFrameRate( const unsigned int& frameRate )
{
	m_desiredRenderTime = 1000.f / ( float ) frameRate;
}

void PointRendererWidget::setJFPBRFirstMaxDistance( double value )
{
	m_renderer->getJumpFlooding().setFirstMaxDistance( ( float )value );
	updateGL();
}

void PointRendererWidget::toggleDrawTrackball( void )
{
	draw_trackball = !draw_trackball;
	updateGL();
}

void PointRendererWidget::toggleDrawAuxViewports( void )
{
	m_drawAuxViewports = !m_drawAuxViewports;
	updateGL();
}

void PointRendererWidget::setJfpbrFrameskip( const int& value )
{
	m_renderer->setJfpbrFrameskip( value );
}
	
void PointRendererWidget::setRenderingTimeTolerance( const int& tolerance )
{
	m_renderingTimeTolerance = tolerance;
}

void PointRendererWidget::openMesh( const string& filename )
{
	Attributes vertAttribs = model::NORMALS;
	
	if( m_octree )
	{
		delete m_octree;
	}
	m_octree = new Octree( 1, 10 );
	m_octree->buildFromFile( filename, PointReader::SINGLE, vertAttribs );
	
	cout << "Octree built." << endl;
	
	mesh.reset();
	if( m_renderer )
	{
		delete m_renderer;
	}
	
	// Render the scene one time, traveling from octree's root to init m_renderTime for future projection
	// threshold adaptations.
	m_renderer = new RenderingState( /*m_octree->getPoints(),*/ &camera, &light_trackball, &mesh, vertAttribs,
									 QApplication::applicationDirPath().toStdString() + "/shaders/tucano/" );
	
	cout << "Renderer built." << endl;
	
	clock_t timing = clock();
	m_octree->traverse( *m_renderer, m_projThresh );
	timing = clock() - timing;
	m_renderTime = float( timing ) / CLOCKS_PER_SEC * 1000;
	
	updateGL();
}