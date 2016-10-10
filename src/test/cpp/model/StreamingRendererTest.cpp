#include <gtest/gtest.h>
#include <utils/qtflycamerawidget.hpp>
#include <QApplication>
#include "PlyPointReader.h"
#include "StreamingRenderer.h"
#include "NodeLoader.h"
#include "OglUtils.h"
#include <GLHiddenWidget.h>

using namespace std;
using namespace util;

namespace model
{
	namespace test
	{
		class StreamingRendererTestWidget
		: public QtFlycameraWidget
		{
		public:
			using Point = model::Point;
			using PointPtr = model::PointPtr;
			using PointArray = Array< PointPtr >;
			using Renderer = StreamingRenderer< Point >;
			using Node = O1OctreeNode< PointPtr >;
			using Siblings = Array< Node >;
			using NodeLoader = model::NodeLoader< Point >;
			
			StreamingRendererTestWidget( NodeLoader& loader, QWidget *parent = 0 )
			: QtFlycameraWidget( parent, loader.widget() ),
			m_loader( loader ),
			m_siblings( 1 )
			{}
			
			~StreamingRendererTestWidget()
			{
				delete m_renderer;
			}
			
			void initialize()
			{
				QtFlycameraWidget::initialize();
				
				PlyPointReader reader( "../data/example/staypuff.ply" );
				m_points = PointArray( reader.getNumPoints() );
				
				Float negInf = -numeric_limits< Float >::max();
				Float posInf = numeric_limits< Float >::max();
				Vec3 origin = Vec3( posInf, posInf, posInf );
				Vec3 maxCoords( negInf, negInf, negInf );
				
				auto iter = m_points.begin();
				reader.read(
					[ & ]( const Point& p )
					{
						*iter++ = make_shared< Point >( p );
						
						const Vec3& pos = p.getPos();
						
						for( int i = 0; i < 3; ++i )
						{
							origin[ i ] = std::min( origin[ i ], pos[ i ] );
							maxCoords[ i ] = std::max( maxCoords[ i ], pos[ i ] );
						}
					}
				);
				
				Vec3 boxSize = maxCoords - origin;
				float scale = 1.f / std::max( std::max( boxSize.x(), boxSize.y() ), boxSize.z() );
				
				for( PointPtr p : m_points )
				{
					Vec3& pos = p->getPos();
					pos = ( pos - origin ) * scale;
				}
				
				m_renderer = new Renderer( camera, &light_trackball, "../shaders/tucano/" );
				OglUtils::checkOglErrors();
				
				m_siblings[ 0 ] = Node( m_points, true );
				OglUtils::checkOglErrors();
				
				m_loader.asyncLoad( m_siblings[ 0 ], 0 );
				m_loader.onIterationEnd();
			}
			
			void paintGL() override
			{
				m_renderer->setupRendering();
				OglUtils::checkOglErrors();
				
				if( !m_siblings.empty() )
				{
					Node& node = m_siblings[ 0 ];
					
					if( node.loadState() == Node::LOADED )
					{
						m_renderer->handleNodeRendering( node );
						m_renderer->afterRendering();
						OglUtils::checkOglErrors();
					}
				}
				
				m_loader.onIterationEnd();
				camera->renderAtCorner();
			}
			
			void keyPressEvent( QKeyEvent * event ) override
			{
				QtFlycameraWidget::keyPressEvent( event );
				
				switch( event->key() )
				{
					case Qt::Key_F1 :
					{
						if( !m_siblings.empty() )
						{
							m_loader.asyncLoad( m_siblings[ 0 ], 0 );
						}
						break;
					}
					case Qt::Key_F2 :
					{
						if( !m_siblings.empty() )
						{
							m_loader.asyncUnload( m_siblings[ 0 ], 0 );
						}
						break;
					}
					case Qt::Key_F3 :
					{
						if( m_siblings.empty() )
						{
							m_siblings = Siblings( 1 );
							m_siblings[ 0 ] = Node( m_points, true );
							m_loader.asyncLoad( m_siblings[ 0 ], 0 );
						}
						break;
					}
					case Qt::Key_F4 :
					{
						m_loader.asyncRelease( std::move( m_siblings ), 0 );
						break;
					}
					
					m_loader.onIterationEnd();
					OglUtils::checkOglErrors();
				}
				
			}
			
		private:
			Renderer* m_renderer;
			NodeLoader& m_loader;
			Siblings m_siblings;
			PointArray m_points;
		};
		
        class StreamingRendererTest : public ::testing::Test
		{
		protected:
			void SetUp() {}
		};
		
		TEST_F( StreamingRendererTest, All )
		{
			{
				GLHiddenWidget hiddenWidget;
				NodeLoaderThread loaderThread( &hiddenWidget, 900ul * 1024ul * 1024ul);
				typename StreamingRendererTestWidget::NodeLoader loader( &loaderThread, 1 );
				
				{
					StreamingRendererTestWidget widget( loader );
					widget.resize( 640, 480 );
					widget.show();
					widget.initialize();

					QApplication::exec();
				}
				
				ASSERT_EQ( 0, loader.memoryUsage() );
			}
			ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
		}
	}
}