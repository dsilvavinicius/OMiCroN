#include "PointRendererWidget.h"
#include "renderers/TucanoDebugRenderer.h"
#include <QDebug>
#include <QTimer>

// #define ADAPTIVE_PROJ

PointRendererWidget::PointRendererWidget( NodeLoader& loader, QWidget *parent )
: Tucano::QtFreecameraWidget( parent, loader.widget() ),
m_projThresh( PROJ_THRESHOLD ),
m_desiredRenderTime( 0.f ),
draw_trackball( true ),
m_drawAuxViewports( false ),
m_octree( nullptr ),
m_renderer( nullptr ),
m_loader( loader ),
m_statistics( m_projThresh ),
m_circlePathFlag( false ),
m_circleT( 0.f )
{
	setlocale( LC_NUMERIC, "C" );
	
	camera->setSpeed( 0.005f );
	m_cameraPath.initialize( "shaders/tucano/" );
	m_cameraPath.setAnimSpeed( CAMERA_PATH_SPEED );
	m_cameraPath.toggleDrawControlPoints();
	loadCameraPath();
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
	
	#if MODEL == DAVID
		#ifdef LAB
			openMesh( "/home/vinicius/Datasets/David/Shallow/David.oct" );
		#else
			openMesh( "/media/vinicius/Expansion Drive3/Datasets/David/Shallow/David.oct" );
		#endif
	#elif MODEL == ST_MATHEW
		#ifdef LAB
			openMesh( "/home/vinicius/Datasets/StMathew/Shallow/StMathew.oct" );
		#else
			openMesh( "/media/vinicius/Expansion Drive3/Datasets/StMathew/Shallow/StMathew.oct" );
		#endif
	#elif MODEL == ATLAS
		#ifdef LAB
			openMesh( "/home/vinicius/Datasets/Atlas/Shallow/Atlas.oct" );
		#else
			openMesh( "/media/vinicius/Expansion Drive3/Datasets/Atlas/Shallow/Atlas.oct" );
		#endif		
	#elif MODEL == DUOMO
		#ifdef LAB
			openMesh( "/home/vinicius/Datasets/Duomo/Shallow/Duomo.oct" );
		#else
			openMesh( "/media/vinicius/Expansion Drive3/Datasets/Duomo/Shallow/Duomo.oct" );
		#endif	
	#endif
	
	m_timer = new QTimer( this );
	connect( m_timer, SIGNAL( timeout() ), this, SLOT( updateGL() ) );
	m_timer->start( 0 ); // Update 60 fps.
}

void PointRendererWidget::resizeGL( int width, int height )
{
	// TODO: It seems that resing is resulting in memory leak ( probably in jump flooding code... ).
	
	camera->setViewport( Eigen::Vector2f( ( float )width, ( float )height ) );
	camera->setPerspectiveMatrix( camera->getFovy(), width / height, 0.001f, 10.0f );
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
	
	if( m_circlePathFlag )
	{
// 		m_circleT += 0.1f * M_PI;
// 		
// 		Vector3f eye(  );
// 		x  =  h + r cos(t)
// 		y  =  k + r sin(t)
// 		
// 		Vector3f circleCenter( 0.f, 0.f, 0.f );
// 		float circleRadius = 2.f;
// 		Affine3f view( Affine3f::Identity() );
// 		AngleAxisf rotation0( M_PI * 0.5, Vector3f::UnitX() );
// 		AngleAxisf rotation1( m_circleT, Vector3f::UnitZ() );
// 		view.rotate( rotation1 * rotation0 );
// 		view.translate( rotation1 * rotation0 * Vector3f( 0.0f, 0.0f, circleRadius ) );
// 		
// 		Flycamera cameraCpy;
// 		cameraCpy.setViewMatrix( view );
// 		
// 		m_cameraPath.addKeyPosition( cameraCpy );
	}
	
	glClearColor( 1.0f, 1.0f, 1.0f, 1.0f ); 
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	makeCurrent();

	#ifdef ADAPTIVE_PROJ
		adaptRenderingThresh( frameTime );
	#endif
	
	// Render the scene.
	auto frontTrackingStart = Profiler::now();
	
	if( m_cameraPath.isAnimating() )	
	{
		m_cameraPath.stepForward();
		camera->setViewMatrix( m_cameraPath.cameraAtCurrentTime().inverse() );
	}
	
	OctreeStats octreeStats = m_octree->trackFront( *m_renderer, m_projThresh );
	
	m_statistics.addFrame( octreeStats, frameTime - m_statistics.m_octreeStats.m_currentStats.m_cpuOverhead );
	
	int frontTrackingTime = Profiler::elapsedTime( frontTrackingStart );
	
	// Render debug data.
	stringstream debugSS;
	debugSS /*<< "Desired render time: " << m_desiredRenderTime << " ms" << endl
			<< "Render time diff: " << frameTime - m_desiredRenderTime << endl
			<< "Rendering time tolerance: " << m_renderingTimeTolerance << " ms" << endl
			<< "Rendering threshold: " << m_projThresh << endl*/
			<< octreeStats << endl;
			
	//cout << debugSS.str() << endl << endl;
	
	int textBoxWidth = width() * 0.3;
	int textBoxHeight = height() * 0.7;
	int margin = 10;
	debugInfoDefined( QString( debugSS.str().c_str() ) );
	
	glEnable(GL_DEPTH_TEST);
	
	if( !m_cameraPath.isAnimating() )
	{
		if( draw_trackball )
		{
			camera->renderAtCorner();
		}
		m_cameraPath.render( *camera, light_trackball );
		
		if( m_drawAuxViewports )
		{
			glEnable( GL_SCISSOR_TEST );
	// 		renderAuxViewport( FRONT );
	// 		renderAuxViewport( SIDE );
	// 		renderAuxViewport( TOP );
			glDisable( GL_SCISSOR_TEST );
		}
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
	Eigen::Vector2f screen_pos( event->x(), event->y() );
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
	m_octree->waitCreation();
	
	time_t now = time(NULL);
	char the_date[ 50 ];
	the_date[0] = '\0';
	
	if (now != -1)
	{
		strftime( the_date, 50, "%d_%m_%Y-%H_%M", localtime( &now ) );
	}
	
	ostringstream statsFilename; statsFilename << "../../statistics/" << m_statistics.m_datasetName << "-" << the_date << ".txt";
	ofstream statsFile( statsFilename.str() );
	
	ostringstream statsString; statsString << m_statistics << endl << "Time to create hierarchy: "
		<< m_octree->hierarchyCreationDuration() << "ms" << endl
		<< "Dynamic memory allocated: " << AllocStatistics::totalAllocated() << " bytes" << endl << endl;
	statsFile << statsString.str();
	
	statsFile.close();
	
	cout << "Statistics saved into " << statsFilename.str() << endl << endl;
	
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
	
	// Main camera control.
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
	
	if( m_keys[ Qt::Key_R ] )
	{
		m_cameraPath.reset();
		m_keys[ Qt::Key_R ] = false;
	}
	if( m_keys[ Qt::Key_K ] )
	{
		m_cameraPath.addKeyPosition( *camera );
		m_keys[ Qt::Key_K ] = false;
	}
	if( m_keys[ Qt::Key_Space ] )
	{
		m_cameraPath.toggleAnimation();
		m_renderer->toggleFboSave();
		m_keys[ Qt::Key_Space ] = false;
	}
	if( m_keys[ Qt::Key_J ] )
	{
		saveCameraPath();
		m_keys[ Qt::Key_J ] = false;
	}
	if( m_keys[ Qt::Key_L ] )
	{
		loadCameraPath();
		m_keys[ Qt::Key_L ] = false;
	}
	if( m_keys[ Qt::Key_I ] )
	{
		m_cameraPath.reset();
		saveScreenshotCamera();
		m_keys[ Qt::Key_I ] = false;
	}
	if( m_keys[ Qt::Key_O ] )
	{
		loadScreenshotCamera();
		m_keys[ Qt::Key_O ] = false;
	}
	
	if( m_keys[ Qt::Key_Enter ] )
	{
		m_circlePathFlag = !m_circlePathFlag;
		camera->updateViewMatrixLookAt();
		m_keys[ Qt::Key_Enter ] = false;
	}
	else
	{
		camera->updateViewMatrix();
	}
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
	
	Octree::RuntimeSetup runtime( HIERARCHY_CREATION_THREADS, WORK_LIST_SIZE, 1024ul * 1024ul * 1024ul * 7ul, true );
	
	if( !filename.substr( filename.find_last_of( '.' ) ).compare( ".oct" ) )
	{
		ifstream file( filename );
		Json::Value octreeJson;
		file >> octreeJson;
		
		// Debug
		{
			cout << "Octree Json " << filename << endl << octreeJson << endl;
		}
		{
			m_statistics.m_hierarchyDepth = octreeJson[ "depth" ].asUInt();
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
	
	{
		int startIdx = filename.find_last_of( '/' ) + 1;
		int endIdx = filename.find_last_of( '.' );
		string modelName = filename.substr( startIdx, endIdx - startIdx );
		m_statistics.m_datasetName = modelName;
	}
	
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

void PointRendererWidget::loadCameraPath()
{
	#if MODEL == DAVID
		m_cameraPath.loadFromFile("../../camera_paths/David");
	#elif MODEL == ST_MATHEW
		m_cameraPath.loadFromFile("../../camera_paths/StMathew");
	#elif MODEL == ATLAS
		m_cameraPath.loadFromFile("../../camera_paths/Atlas");
	#elif MODEL == DUOMO
		m_cameraPath.loadFromFile("../../camera_paths/Duomo");
	#endif
}
	
void PointRendererWidget::saveCameraPath()
{
	#if MODEL == DAVID
		m_cameraPath.writeToFile("../../camera_paths/David");
	#elif MODEL == ST_MATHEW
		m_cameraPath.writeToFile("../../camera_paths/StMathew");
	#elif MODEL == ATLAS
		m_cameraPath.writeToFile("../../camera_paths/Atlas");
	#elif MODEL == DUOMO
		m_cameraPath.writeToFile("../../camera_paths/Duomo");
	#endif
}

void PointRendererWidget::loadScreenshotCamera()
{
	#if MODEL == DAVID
		m_cameraPath.loadFromFile("../../screenshot_cameras/David");
	#elif MODEL == ST_MATHEW
		m_cameraPath.loadFromFile("../../screenshot_cameras/StMathew");
	#elif MODEL == ATLAS
		m_cameraPath.loadFromFile("../../screenshot_cameras/Atlas_screenshot");
	#elif MODEL == DUOMO
		m_cameraPath.loadFromFile("../../screenshot_cameras/Duomo_screenshot");
	#endif
}
	
void PointRendererWidget::saveScreenshotCamera()
{
	m_cameraPath.addKeyPosition( *camera );
	
	#if MODEL == DAVID
		m_cameraPath.writeToFile("../../screenshot_cameras/David");
	#elif MODEL == ST_MATHEW
		m_cameraPath.writeToFile("../../screenshot_cameras/StMathew");
	#elif MODEL == ATLAS
		m_cameraPath.writeToFile("../../screenshot_cameras/Atlas");
	#elif MODEL == DUOMO
		m_cameraPath.writeToFile("../../screenshot_cameras/Duomo");
	#endif
}