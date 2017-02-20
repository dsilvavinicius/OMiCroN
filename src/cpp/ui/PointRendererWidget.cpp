#include "PointRendererWidget.h"
#include "renderers/TucanoDebugRenderer.h"
#include <QDebug>
#include <QTimer>

// #define ADAPTIVE_PROJ

PointRendererWidget::PointRendererWidget( NodeLoader& loader, QWidget *parent )
: Tucano::QtFreecameraWidget( parent, loader.widget() ),
m_projThresh( 0.2f ),
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
	Tucano::QtFreecameraWidget::initialize();
	
	setFrameRate( frameRate );
	m_renderingTimeTolerance = renderingTimeTolerance;
	
// 	openMesh( QDir::currentPath().append( "/data/example/staypuff.ply" ).toStdString() );
// 	openMesh( QDir::currentPath().append( "/data/example/sorted_staypuff.oct" ).toStdString() );
	
	#ifdef DAVID
		openMesh( "/media/vinicius/Expansion Drive3/Datasets/David/Shallow/David.oct" );
// 		openMesh( "/home/lcg/vinicius/Datasets/Shallow/David.oct" );
	#elif defined ST_MATHEW
		openMesh( "/media/vinicius/Expansion Drive3/Datasets/StMathew/Shallow/StMathew.oct" );
// 		openMesh( "/home/lcg/vinicius/Datasets/StMathew/Shallow/StMathew.oct" );
	#elif defined ATLAS
		openMesh( "/media/vinicius/Expansion Drive3/Datasets/Atlas/Shallow/Atlas.oct" );
// 		openMesh( "/home/lcg/vinicius/Datasets/Atlas/Shallow/Atlas.oct" );
	#elif defined DUOMO
		openMesh( "/media/vinicius/Expansion Drive3/Datasets/Duomo/Shallow/Duomo.oct" );
// 		openMesh( "/home/lcg/vinicius/Datasets/Duomo/Shallow/Duomo.oct" );
	#endif
	
	m_timer = new QTimer( this );
	connect( m_timer, SIGNAL( timeout() ), this, SLOT( updateGL() ) );
	m_timer->start( 0 ); // Update 60 fps.
}

void PointRendererWidget::resizeGL( int width, int height )
{
	// TODO: It seems that resing is resulting in memory leak ( probably in jump flooding code... ).
	
	camera->setViewport( Eigen::Vector2f( ( float )width, ( float )height ) );
	camera->setPerspectiveMatrix( camera->getFovy(), width / height, 0.001f, 1.0f );
	light_trackball.setViewport( Eigen::Vector2f( ( float )width, ( float )height ) );

	if( m_renderer )
	{
		m_renderer->reshape( width, height );
	}
	
	updateGL();
}


void PointRendererWidget::adaptRenderingThresh( const float renderTime )
{
	float renderTimeDiff = renderTime - m_desiredRenderTime;
	
	if( fabs( renderTimeDiff ) > m_renderingTimeTolerance )
	{
		float projIncrement = renderTimeDiff * 1e-3;
		m_projThresh += projIncrement;
		m_projThresh = std::max( m_projThresh, 0.001953125f ); // 2 / 1024. So it is expected a screen of 1024 pixels.
		m_projThresh = std::min( m_projThresh, 1.f );
	}
}

void PointRendererWidget::paintGL (void)
{
	//cout << "=== Painting starts ===" << endl << endl;
	
	int frameTime = Profiler::elapsedTime( m_beginOfFrameTime );
	m_beginOfFrameTime = Profiler::now();
	
	updateFromKeyInput();
	
	glClearColor( 1.0f, 1.0f, 1.0f, 1.0f ); 
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	makeCurrent();

	#ifdef ADAPTIVE_PROJ
		adaptRenderingThresh( frameTime );
	#endif
	
	// Render the scene.
	auto frontTrackingStart = Profiler::now();
	
	OctreeStats octreeStats = m_octree->trackFront( *m_renderer, m_projThresh );
	
	m_statistics.addFrame( octreeStats, frameTime - m_statistics.m_octreeStats.m_currentStats.m_cpuOverhead );
	
	int frontTrackingTime = Profiler::elapsedTime( frontTrackingStart );
	
	// Render debug data.
	stringstream debugSS;
	debugSS /*<< "Desired render time: " << m_desiredRenderTime << " ms" << endl
			<< "Render time diff: " << frameTime - m_desiredRenderTime << endl
			<< "Rendering time tolerance: " << m_renderingTimeTolerance << " ms" << endl
			<< "Rendering threshold: " << m_projThresh << endl*/
			<< m_statistics << endl;
			
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
	m_keys[ event->key() ] = true;
}

void PointRendererWidget::keyReleaseEvent( QKeyEvent * event )
{
	m_keys[ event->key() ] = false;
}

void PointRendererWidget::mousePressEvent( QMouseEvent * event )
{
	setFocus ();
	Eigen::Vector2f screen_pos (event->x(), event->y());
	if (event->button() == Qt::LeftButton)
	{
		camera->startRotation(screen_pos);
		camera->updateViewMatrix();
	}
	if (event->button() == Qt::MiddleButton)
	{
		camera->startRotation(screen_pos);
		camera->updateViewMatrix();
	}
	if (event->button() == Qt::RightButton)
	{
		light_trackball.rotateCamera(screen_pos);
	}
}

void PointRendererWidget::mouseMoveEvent( QMouseEvent * event )
{
	Eigen::Vector2f screen_pos (event->x(), event->y());
	if (event->buttons() & Qt::LeftButton)
	{
		camera->rotate(screen_pos);
		camera->updateViewMatrix();
	}
	if(event->buttons() & Qt::MiddleButton)
	{
		camera->rotateZ(screen_pos);
		camera->updateViewMatrix();
	}
	if (event->buttons() & Qt::RightButton)
	{
		light_trackball.rotateCamera(screen_pos);
	}
}

void PointRendererWidget::mouseReleaseEvent( QMouseEvent * event )
{
	if (event->button() == Qt::RightButton)
	{
		light_trackball.endRotation();
	}
}

void PointRendererWidget::closeEvent( QCloseEvent * event )
{
	stringstream ss; ss << m_statistics << endl << "Dynamic memory allocated: " << AllocStatistics::totalAllocated() << endl << endl;
	HierarchyCreationLog::logDebugMsg( ss.str() );
	HierarchyCreationLog::flush();
	
	cout << "Statistics saved." << endl << endl;
	
	event->accept();
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
}

void PointRendererWidget::toggleEffect( int id )
{
}

void PointRendererWidget::reloadShaders( void )
{
}

void PointRendererWidget::setFrameRate( const unsigned int& frameRate )
{
	m_desiredRenderTime = 1000.f / ( float ) frameRate;
}

void PointRendererWidget::setJFPBRFirstMaxDistance( double value )
{
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
	updateGL();
}

void PointRendererWidget::setJfpbrFrameskip( const int& value )
{
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
	
	Octree::RuntimeSetup runtime( nThreads, 8/*32*/, 1024ul * 1024ul * 1024ul * 7ul, true );
	
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
	
	m_beginOfFrameTime = Profiler::now();
	
	m_octree->trackFront( *m_renderer, m_projThresh );
	
	updateGL();
}