#include "PointRendererWidget.h"
#include "renderers/TucanoDebugRenderer.h"
#include <QDebug>
#include <QTimer>

#define ADAPTIVE_PROJ

PointRendererWidget::PointRendererWidget( NodeLoader& loader, QWidget *parent )
: Tucano::QtFreecameraWidget( parent, loader.widget() ),
m_projThresh( 1.f ),
m_renderTime( 0.f ),
m_desiredRenderTime( 0.f ),
draw_trackball( true ),
m_drawAuxViewports( false ),
m_octree( nullptr ),
m_renderer( nullptr ),
m_loader( loader )
{
	setlocale( LC_NUMERIC, "C" );
	
	camera->setSpeed( 0.005f );
}

PointRendererWidget::~PointRendererWidget()
{
	delete m_renderer;
	delete m_octree;
	delete m_timer;
}

void PointRendererWidget::initialize( const unsigned int& frameRate, const int& renderingTimeTolerance )
{
	// Init MemoryManager allowing 9GB of data.
// 	DefaultManager< MortonCode, Point, OctreeNode >::initInstance( 1024ul * 1024ul * 1024ul * 9 );
	
	//Ken12MemoryManager< MortonCode, Point, InnerNode, LeafNode >::initInstance(
	//	1.5f * 1024ul * 1024ul * 1024ul / sizeof( MortonCode ) /* 1.5GB for MortonCodes */,
	//	3.25f * 1024ul * 1024ul * 1024ul / sizeof( Point ) /* 3.25GB for Points */,
	//	1.625f * 1024ul * 1024ul * 1024ul / sizeof( InnerNode ) /* 1.625GB for Nodes */,
	//	1.625f * 1024ul * 1024ul * 1024ul / sizeof( LeafNode ) /* 1.625GB for Nodes */
	//);
	
// 	cout << "MemoryManager initialized: " << endl << SingletonMemoryManager::instance() << endl;
	
	Tucano::QtFreecameraWidget::initialize();
	
	setFrameRate( frameRate );
	m_renderingTimeTolerance = renderingTimeTolerance;
	
// 	openMesh( QDir::currentPath().append( "/data/example/staypuff.ply" ).toStdString() );
// 	openMesh( QDir::currentPath().append( "/data/example/sorted_staypuff.oct" ).toStdString() );
// 	openMesh( "/media/vinicius/Expansion Drive3/Datasets/David/test/test.oct" );
	openMesh( "/media/vinicius/Expansion Drive3/Datasets/David/Shallow/David.oct" );
	
	m_timer = new QTimer( this );
	connect( m_timer, SIGNAL( timeout() ), this, SLOT( updateGL() ) );
	m_timer->start( 0 ); // Update 60 fps.
}

void PointRendererWidget::resizeGL( int width, int height )
{
	// TODO: It seems that resing is resulting in memory leak ( probably in jump flooding code... ).
	
	camera->setViewport( Eigen::Vector2f( ( float )width, ( float )height ) );
	camera->setPerspectiveMatrix( camera->getFovy(), width / height, 0.001f, 500.0f );
	light_trackball.setViewport( Eigen::Vector2f( ( float )width, ( float )height ) );

	if( m_renderer )
	{
		m_renderer->reshape( width, height );
	}
	
	updateGL();
}


void PointRendererWidget::adaptRenderingThresh()
{
	float renderTimeDiff = m_renderTime - m_desiredRenderTime;
	
	if( abs( renderTimeDiff ) > m_renderingTimeTolerance )
	{
		m_projThresh += renderTimeDiff * 1.0e-1f;
		m_projThresh = std::max( m_projThresh, 1.f );
		m_projThresh = std::min( m_projThresh, 500.f );
	}
}

void PointRendererWidget::paintGL (void)
{
	//cout << "=== Painting starts ===" << endl << endl;
	
	updateFromKeyInput();
	
	glClearColor( 1.0f, 1.0f, 1.0f, 1.0f ); 
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
// 	glClearColor( 1.0, 1.0, 1.0, 1.0 );
// 	glClearDepth( 1.0 );
// 	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
// 	if( Profiler::elapsedTime( m_inputEndTime ) > 1000 )
// 	{
// 		m_renderer->selectEffect( Renderer::JUMP_FLOODING );
// 	}
	
	auto frameStart = Profiler::now();
	makeCurrent();

	#ifdef ADAPTIVE_PROJ
		adaptRenderingThresh();
	#endif
	
	// Render the scene.
	auto frontTrackingStart = Profiler::now();
	
	FrontOctreeStats stats = m_octree->trackFront( *m_renderer, m_projThresh );
	
	int frontTrackingTime = Profiler::elapsedTime( frontTrackingStart );
	
	m_renderTime = Profiler::elapsedTime( frameStart );
	
	// Render debug data.
	stringstream debugSS;
	debugSS << "Total frame time: " << m_renderTime << " ms" << endl << endl
			<< "Render time (traversal + rendering): " << frontTrackingTime << " ms" << endl << endl
			<< "Time between frames: " << Profiler::elapsedTime( m_endOfFrameTime, frameStart ) << "ms" << endl
			<< "Desired render time: " << m_desiredRenderTime << " ms" << endl << endl
			<< "Render time diff: " << m_renderTime - m_desiredRenderTime << endl << endl
			<< "Rendering time tolerance: " << m_renderingTimeTolerance << " ms" << endl << endl
			<< "Rendering threshold: " << m_projThresh << endl << endl
			<< stats;
			
	//cout << debugSS.str() << endl << endl;
	
	int textBoxWidth = width() * 0.3;
	int textBoxHeight = height() * 0.7;
	int margin = 10;
	debugInfoDefined( QString( debugSS.str().c_str() ) );
	
	glEnable(GL_DEPTH_TEST);
	if( draw_trackball )
	{
		camera->renderAtCorner();
	}
	
	m_endOfFrameTime = Profiler::now();
	
	if( m_drawAuxViewports )
	{
		glEnable( GL_SCISSOR_TEST );
// 		renderAuxViewport( FRONT );
// 		renderAuxViewport( SIDE );
// 		renderAuxViewport( TOP );
		glDisable( GL_SCISSOR_TEST );
	}
	
	//cout << "=== Painting ends ===" << endl << endl;
}

void PointRendererWidget::renderAuxViewport( const Viewport& viewport )
{
// 	Vector2f viewportPos;
// 	
// 	switch( viewport )
// 	{
// 		case FRONT: viewportPos[ 0 ] = 0.f; viewportPos[ 1 ] = 0.f; break;
// 		case SIDE: viewportPos[ 0 ] = size().width() * 0.333f; viewportPos[ 1 ] = 0.f; break;
// 		case TOP: viewportPos[ 0 ] = size().width() * 0.666f; viewportPos[ 1 ] = 0.f; break;
// 	}
// 	
// 	Vector4f auxViewportSize( viewportPos[ 0 ], viewportPos[ 1 ], size().width() * 0.333f, size().height() * 0.333f );
// 	glScissor( auxViewportSize.x(), auxViewportSize.y(), auxViewportSize.z(), auxViewportSize.w() );
// 	glClear( GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT );
// 	
// 	Trackball tempCamera;
// 	tempCamera.setViewport( auxViewportSize );
// 	tempCamera.setPerspectiveMatrix( tempCamera.getFovy(), auxViewportSize.z() / auxViewportSize.w(), 0.1f, 10000.0f );
// 	tempCamera.resetViewMatrix();
// 	
// 	switch( viewport )
// 	{
// 		case FRONT:
// 		{
// 			 tempCamera.translate( Vector3f( 0.f, 0.f, -200.f ) );
// 			 break;
// 		}
// 		case SIDE:
// 		{
// 			tempCamera.rotate( Quaternionf( AngleAxisf( 0.5 * M_PI, Vector3f::UnitY() ) ) );
// 			tempCamera.translate( Vector3f( 200.f, 0.f, 0.f ) );
// 			break;
// 		}
// 		case TOP:
// 		{
// 			tempCamera.rotate( Quaternionf( AngleAxisf( 0.5 * M_PI, Vector3f::UnitX() ) ) );
// 			tempCamera.translate( Vector3f( 0.f, -200.f, 0.f ) );
// 			break;
// 		}
// 	}
// 	
// 	
// 	Phong &phong = m_renderer->getPhong();
// 	phong.render( mesh, tempCamera, light_trackball );
}

void PointRendererWidget::keyPressEvent( QKeyEvent * event )
{
	m_inputEndTime = Profiler::now();
// 	m_renderer->selectEffect( Renderer::PHONG );
	m_keys[ event->key() ] = true;
}

void PointRendererWidget::keyReleaseEvent( QKeyEvent * event )
{
	m_inputEndTime = Profiler::now();
// 	m_renderer->selectEffect( Renderer::PHONG );
	m_keys[ event->key() ] = false;
}

void PointRendererWidget::mousePressEvent( QMouseEvent * event )
{
	m_inputEndTime = Profiler::now();
// 	m_renderer->selectEffect( Renderer::PHONG );
	QtFreecameraWidget::mousePressEvent( event );
}

void PointRendererWidget::mouseMoveEvent( QMouseEvent * event )
{
	m_inputEndTime = Profiler::now();
// 	m_renderer->selectEffect( Renderer::PHONG );
	QtFreecameraWidget::mouseMoveEvent( event );
}

void PointRendererWidget::mouseReleaseEvent( QMouseEvent * event )
{
	m_inputEndTime = Profiler::now();
// 	m_renderer->selectEffect( Renderer::PHONG );
	QtFreecameraWidget::mouseReleaseEvent( event );
}

void PointRendererWidget::updateFromKeyInput()
{
	if( m_keys[ Qt::Key_O ] )
	{
		QString filename = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("Mesh Files (*.obj *.ply)"));
		if (!filename.isEmpty())
		{
			openMesh (filename.toStdString());
		}
	}
	if( m_keys[ Qt::Key_R ] )
		camera->reset();
	if( m_keys[ Qt::Key_A ] )
		camera->strideLeft();
	if( m_keys[ Qt::Key_D ] )
		camera->strideRight();
	if( m_keys[ Qt::Key_S ] )
		camera->moveBack();
	if( m_keys[ Qt::Key_W ] )
		camera->moveForward();
	if( m_keys[ Qt::Key_C ] )
		camera->moveDown();
	if( m_keys[ Qt::Key_E ] )
		camera->moveUp();	

	camera->updateViewMatrix();
}

void PointRendererWidget::toggleWriteFrames()
{
// 	m_renderer->getJumpFlooding().toggleWriteFrames();	
// 	updateGL();
}

void PointRendererWidget::toggleEffect( int id )
{
// 	m_renderer->selectEffect( ( Renderer::Effect ) id );
// 	updateGL();
}

void PointRendererWidget::reloadShaders( void )
{
// 	m_renderer->getPhong().reloadShaders();
// 	m_renderer->getJumpFlooding().reloadShaders();
// 	updateGL();
}

void PointRendererWidget::setFrameRate( const unsigned int& frameRate )
{
	m_desiredRenderTime = 1000.f / ( float ) frameRate;
}

void PointRendererWidget::setJFPBRFirstMaxDistance( double value )
{
// 	m_renderer->getJumpFlooding().setFirstMaxDistance( ( float )value );
// 	updateGL();
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

void PointRendererWidget::toggleNodeDebugDraw( const int& value )
{
// 	m_octree->toggleDebug( value );
	updateGL();
}

void PointRendererWidget::setJfpbrFrameskip( const int& value )
{
// 	m_renderer->setJfpbrFrameskip( value );
}
	
void PointRendererWidget::setRenderingTimeTolerance( const int& tolerance )
{
	m_renderingTimeTolerance = tolerance;
}

void PointRendererWidget::openMesh( const string& filename )
{
	if( m_octree )
	{
		delete m_octree;
	}
	
	// Debug value.
// 	int nThreads = 1;
	
	// Best value for performance
	int nThreads = 4;
	
	Octree::RuntimeSetup runtime( nThreads, 4/*32*/, 1024ul * 1024ul * 1024ul * 12ul, true );
	
	if( !filename.substr( filename.find_last_of( '.' ) ).compare( ".oct" ) )
	{
		ifstream file( filename );
		Json::Value octreeJson;
		file >> octreeJson;
		
		// Debug
		{
			cout << "Octree Json " << filename << endl << octreeJson << endl;
		}
		
		m_octree = new Octree( octreeJson, m_loader, runtime );
	}
	else if( !filename.substr( filename.find_last_of( '.' ) ).compare( ".ply" ) )
	{
		m_octree = new Octree( filename, 5, m_loader, runtime );
	}
	else
	{
		throw runtime_error( "Supported file formats are .oct for already processed octrees or .ply for raw point clouds." );
	}
	
	cout << "Octree built." << endl;
	
	mesh.reset();
	if( m_renderer )
	{
		delete m_renderer;
	}
	
	// Render the scene one time, traveling from octree's root to init m_renderTime for future projection
	// threshold adaptations.
	m_renderer = new Renderer( camera/**, &light_trackball, "shaders/tucano/"*/ );
	
	cout << "Renderer built." << endl;
	
	auto frontTrackingStart = Profiler::now();
	
	m_octree->trackFront( *m_renderer, m_projThresh );
	updateGL();
	
	m_endOfFrameTime = Profiler::now();
	m_renderTime = Profiler::elapsedTime( frontTrackingStart, m_endOfFrameTime );
}