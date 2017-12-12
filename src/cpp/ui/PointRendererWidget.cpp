#include "PointRendererWidget.h"
#include "renderers/TucanoDebugRenderer.h"
#include <OctreeFile.h>
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
m_statistics( m_projThresh )
{
	setlocale( LC_NUMERIC, "C" );
	
	camera->setSpeed( 0.005f );
	m_cameraPath.initialize( "shaders/tucano/" );
	m_cameraPath.setAnimSpeed( CAMERA_PATH_SPEED );
	m_cameraPath.toggleDrawControlPoints();
// 	m_cameraPath.toggleDrawQuaternions();
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
			#if NO_SORT == true
				openMesh( "/media/viniciusdasilva/Expansion Drive/Datasets/David/Shallow/David_lab.oct" );
			#else
				openMesh( "/home/vinicius/Datasets/David/David.ply" );
			#endif
		#else
			#if NO_SORT == true
				openMesh( "/media/vinicius/data/Datasets/David/DavidWithFaces_sorted7.oct" );
			#else
				openMesh( "/media/vinicius/data/Datasets/David/DavidWithFaces.ply" );
			#endif
		#endif
	#elif MODEL == ST_MATHEW
		#ifdef LAB
			openMesh( "/media/viniciusdasilva/Expansion Drive/Datasets/StMathew/Shallow/StMathew_lab.oct" );
		#else
			#if NO_SORT == true
				openMesh( "/media/vinicius/data/Datasets/StMathew/StMathewWithFaces_sorted7.oct" );
			#else
				openMesh( "/media/vinicius/data/Datasets/StMathew/StMathewWithFaces.ply" );
			#endif
		#endif
	#elif MODEL == ATLAS
		#ifdef LAB
			openMesh( "/media/viniciusdasilva/Expansion Drive/Datasets/Atlas/Shallow/Atlas_lab.oct" );
		#else
			#if NO_SORT == true
				openMesh( "/media/vinicius/data/Datasets/Atlas/AtlasWithFaces_sorted7.oct" );
			#else
				openMesh( "/media/vinicius/data/Datasets/Atlas/AtlasWithFaces.ply" );
			#endif
		#endif		
	#elif MODEL == DUOMO
		#ifdef LAB
			openMesh( "/media/viniciusdasilva/Expansion Drive/Duomo/Shallow/Duomo_lab.oct" );
		#else
			openMesh( "/media/vinicius/data/Datasets/Duomo/DuomoWithFaces_sorted7.oct" );
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
// 		if( !m_cameraPath.isAnimationAtEnd() )
// 		{
			m_cameraPath.stepForward();
// 		}
		camera->setViewMatrix( m_cameraPath.cameraAtCurrentTime().inverse() );
	}
	
	OctreeStats octreeStats = m_octree->trackFront( *m_renderer, m_projThresh );
	
	m_statistics.addFrame( octreeStats, frameTime - m_statistics.m_octreeStats.m_currentStats.m_cpuOverhead );
	
	int frontTrackingTime = Profiler::elapsedTime( frontTrackingStart );
	
	{
		float currentCompletion = m_statistics.currentCompletion();
		if( m_octree->substitutedPlaceholders() >= EXPECTED_SUBSTITUTED_PLACEHOLDERS * ( currentCompletion + 0.1f )
			|| ( m_octree->substitutedPlaceholders() == EXPECTED_SUBSTITUTED_PLACEHOLDERS && m_statistics.currentCompletion() < 1.f ) )
		{
			m_statistics.addCompletionPercent( m_statistics.currentCompletion() + 0.1f );
		}
	}
	
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
	
	if( m_octSaveFuture.valid() )
	{
		cout << "Waiting for pending save octree operation." << endl << endl;
		m_octSaveFuture.get();
	}
	
	time_t now = time(NULL);
	char the_date[ 50 ];
	the_date[0] = '\0';
	
	if (now != -1)
	{
		strftime( the_date, 50, "%d_%m_%Y-%H_%M", localtime( &now ) );
	}
	
	cout << "Generating statistics..." << endl << endl;
	
	ostringstream statsFilename; statsFilename << "../../statistics/" << m_statistics.m_datasetName << "-" << the_date << ".txt";
	ofstream statsFile( statsFilename.str() );
	
	pair< uint, uint > nodeStats = m_octree->nodeStatistics();
	
	ostringstream statsString; statsString << m_statistics << endl
		<< "No sort? " << ( ( NO_SORT ) ? "true" : "false" ) << endl
		<< "Sorting flag: " << Sorting( SORTING ) << endl
		<< "Sorting chunks: " << SORTING_SEGMENTS << endl
		<< "Time for reader input: " << m_octree->readerInTime() << "ms" << endl
		<< "Time for reader init: " << m_octree->readerInitTime() << "ms" << endl
		<< "Time for reader reading: " << m_octree->readerReadTime() << "ms" << endl
		<< "Time to create hierarchy: " << m_octree->hierarchyCreationDuration() << "ms" << endl
		<< "Dynamic memory allocated: " << AllocStatistics::totalAllocated() << " bytes" << endl
		<< "Number of nodes in hierarchy: " << nodeStats.first << endl 
		<< "Number of splats in hierarchy: " << nodeStats.second << endl
		<< "Number of substituted placeholders: " << m_octree->substitutedPlaceholders() << endl << endl;
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
		m_renderer->toggleModelMatrixUse();
		m_renderer->toggleFboSave();
		m_keys[ Qt::Key_Enter ] = false;
	}
	else
	{
		camera->updateViewMatrix();
	}
}

void PointRendererWidget::setFrameRate( const unsigned int& frameRate )
{
	m_desiredRenderTime = 1000.f / ( float ) frameRate;
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
	
	RuntimeSetup runtime( HIERARCHY_CREATION_THREADS, WORK_LIST_SIZE, RAM_QUOTA );
	
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
		m_octree = new Octree( filename, 7, m_loader, runtime );
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
	Vector3f centroid = m_octree->dim().m_size * 0.5f;
	
	cout << "Model centroid: " << centroid << endl << "Model origin: " << m_octree->dim().m_origin << endl << endl;
	
	m_renderer = new Renderer( camera, centroid );
	
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
		m_cameraPath.loadFromFile("../../screenshot_cameras/Atlas");
	#elif MODEL == DUOMO
		m_cameraPath.loadFromFile("../../screenshot_cameras/Duomo");
	#endif
}
	
void PointRendererWidget::saveScreenshotCamera()
{
	m_cameraPath.addKeyPosition( *camera );
	m_cameraPath.addKeyPosition( *camera ); // Spaguetti needed.
	
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

void PointRendererWidget::saveOctree()
{
	if( m_octree->isCreationFinished() )
	{
		if( m_octSaveFuture.valid() )
		{
			cout << "Save octree operation pending. Wait for previous one to finish." << endl << endl;
		}
		else
		{
			packaged_task< int () > task(
				[ & ]
				{
					auto now = Profiler::now( "Save octree operation" );
					
					#if MODEL == DAVID
						#ifdef LAB
							string filename = "/media/viniciusdasilva/Expansion Drive/Datasets/David/David.boc";
						#else
							string filename = "/media/vinicius/data/Datasets/David/David.boc";
						#endif
					#elif MODEL == ST_MATHEW
						#ifdef LAB
							string filename = "/media/viniciusdasilva/Expansion Drive/Datasets/StMathew/StMathew.boc";
						#else
							string filename = "/media/vinicius/data/Datasets/StMathew/StMathew.boc";
						#endif
					#elif MODEL == ATLAS
						#ifdef LAB
							string filename = "/media/viniciusdasilva/Expansion Drive/Datasets/Atlas/Atlas.boc";
						#else
							string filename = "/media/vinicius/data/Datasets/Atlas/Atlas.boc";
						#endif		
					#elif MODEL == DUOMO
						#ifdef LAB
							string filename = "/media/viniciusdasilva/Expansion Drive/Datasets/Duomo/Duomo.oct";
						#else
							string filename = "/media/vinicius/data/Datasets/Duomo/Duomo.boc";
						#endif	
					#endif
					
					OctreeFile::write( filename, m_octree->root() );
					
					return Profiler::elapsedTime( now, "Save octree operation" );
				}
			);
			m_octSaveFuture = task.get_future();
			std::thread t( std::move( task ) );
			t.detach();
		}
	}
}