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
			using NodeLoader = model::NodeLoader< Point >;
			
			StreamingRendererTestWidget( NodeLoader& loader, QWidget *parent = 0 )
			: QtFlycameraWidget( parent, loader.widget() ),
			m_loader( loader )
			{}
			
			~StreamingRendererTestWidget()
			{
				delete m_renderer;
			}
			
			void initialize()
			{
				QtFlycameraWidget::initialize();
				
				PlyPointReader< Point > reader( "../data/example/staypuff.ply" );
				PointArray points( reader.getNumPoints() );
				
				Float negInf = -numeric_limits< Float >::max();
				Float posInf = numeric_limits< Float >::max();
				Vec3 origin = Vec3( posInf, posInf, posInf );
				Vec3 maxCoords( negInf, negInf, negInf );
				
				auto iter = points.begin();
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
				
				for( PointPtr p : points )
				{
					Vec3& pos = p->getPos();
					pos = ( pos - origin ) * scale;
				}
				
				m_renderer = new Renderer( camera, &light_trackball, "../shaders/tucano/" );
				OglUtils::checkOglErrors();
				
				m_node = Node( points, true );
				OglUtils::checkOglErrors();
			}
			
			void paintGL() override
			{
				if( m_node.loadState() == Node::UNLOADED )
				{
					m_loader.asyncLoad( m_node, 0 );
					OglUtils::checkOglErrors();
					
					m_loader.onIterationEnd();
					OglUtils::checkOglErrors();
				}
				
				m_renderer->setupRendering();
				
				OglUtils::checkOglErrors();
				
				m_renderer->handleNodeRendering( m_node );
				m_renderer->afterRendering();
				
				OglUtils::checkOglErrors();
				
				camera->renderAtCorner();
			}
			
		private:
			Renderer* m_renderer;
			NodeLoader& m_loader;
			Node m_node;
		};
		
        class StreamingRendererTest : public ::testing::Test
		{
		protected:
			void SetUp() {}
		};
		
		TEST_F( StreamingRendererTest, All )
		{
			GLHiddenWidget hiddenWidget;
			NodeLoaderThread loaderThread( &hiddenWidget, 900ul * 1024ul * 1024ul);
			typename StreamingRendererTestWidget::NodeLoader loader( &loaderThread, 1 );
			
			StreamingRendererTestWidget widget( loader );
			widget.resize( 640, 480 );
			widget.show();
			widget.initialize();

			QApplication::exec();
		}
	}
}