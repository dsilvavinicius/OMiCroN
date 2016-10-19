#include <gtest/gtest.h>
#include <utils/qtfreecamerawidget.hpp>
#include <QApplication>
#include "PlyPointReader.h"
#include "splat_renderer/splat_renderer.hpp"
// #include "splat_renderer/surfel.hpp"

#include "OglUtils.h"
#include <PlyPointWritter.h>
#include "phongshader.hpp"

using namespace std;
using namespace util;
using namespace Tucano;

namespace model
{
	namespace test
	{
		class SplatRendererTestWidget
		: public QtFreecameraWidget
		{
		public:
			using SurfelVector = vector< Surfel >;
			
			SplatRendererTestWidget( QWidget *parent = 0 )
			: QtFreecameraWidget( parent )
			, m_renderer( nullptr )
			{}
			
			~SplatRendererTestWidget()
			{
				delete m_renderer;
			}
			
			void initialize()
			{
				QtFreecameraWidget::initialize();
				
				PlyPointReader reader( "../data/example/staypuff.ply" );
				
				Float negInf = -numeric_limits< Float >::max();
				Float posInf = numeric_limits< Float >::max();
				Vec3 origin = Vec3( posInf, posInf, posInf );
				Vec3 maxCoords( negInf, negInf, negInf );
				
				reader.read(
					[ & ]( const Point& p )
					{
						const Vec3& pos = p.getPos();
						const Vector3f& normal = p.getNormal();
						Vector3f pointOnPlane(
							( normal.x() * pos.x() + normal.y() * pos.y() + normal.z() * pos.z() ) / normal.x(),
							0.f, 0.f );
						Vector3f u = pointOnPlane - pos;
						u.normalize();
						Vector3f v = normal.cross( u );
			
						u *= 0.003f;
						v *= 0.003f;
						
						Surfel s( pos, u, v, Vector3f::Zero(), 255 );
						m_surfels.push_back( s );
						
						for( int i = 0; i < 3; ++i )
						{
							origin[ i ] = std::min( origin[ i ], pos[ i ] );
							maxCoords[ i ] = std::max( maxCoords[ i ], pos[ i ] );
						}
					}
				);
				
				Vec3 boxSize = maxCoords - origin;
				float scale = 1.f / std::max( std::max( boxSize.x(), boxSize.y() ), boxSize.z() );
				Vec3 midPoint = ( origin + maxCoords ) * 0.5f;
				
				for( Surfel& surfel : m_surfels )
				{
					surfel.c = ( surfel.c - midPoint ) * scale;
				}
				
// 				vector< Vector4f > vertices( m_surfels.size() );
// 				vector< Vector3f > normals( m_surfels.size() );
// 				vector< Vector4f > colors( m_surfels.size() );
// 				
// 				auto verticesIter = vertices.begin();
// 				auto normalsIter = normals.begin();
// 				auto colorsIter = colors.begin();
// 				for( Surfel& surfel : m_surfels )
// 				{
// 					*verticesIter++ = Vector4f( surfel.c.x(), surfel.c.y(), surfel.c.z(), 1.0f );
// 					*normalsIter++ = surfel.u.cross( surfel.v );
// 					*colorsIter++ = Vector4f( 1.f, 1.f, 1.f, 0.f );
// 				}
				
// 				PlyPointWritter writer( reader, "staypuff_temp.ply", reader.getNumPoints() );
// 				
// 				verticesIter = vertices.begin();
// 				normalsIter = normals.begin();
// 				while( verticesIter != vertices.end() )
// 				{
// 					Vector3f pos( verticesIter->x(), verticesIter->y(), verticesIter->z() );
// 					Point p( *normalsIter, pos );
// 					
// 					writer.write( p );
// 					
// 					verticesIter++;
// 					normalsIter++;
// 				}
				
// 				m_mesh.loadVertices( vertices );
// 				m_mesh.loadNormals( normals );
// 				m_mesh.loadColors( colors );
// 				
// 				m_phong.setShadersDir( "../shaders/tucano/" );
// 				m_phong.initialize();
				
// 				camera->translate( Eigen::Vector3f( 0.0f, 0.0f, -2.0f ) );
				m_renderer = new SplatRenderer( *camera );
				m_renderer->load_to_gpu( m_surfels );
				OglUtils::checkOglErrors();
			}
			
			void resizeGL (int w, int h) override
			{
				QtFreecameraWidget::resizeGL( w, h );
				
				if( m_renderer != nullptr )
				{
					m_renderer->reshape( w, h );
				}
			}
			
			void paintGL() override
			{
				if( m_renderer != nullptr )
				{
					m_renderer->render_frame();
					OglUtils::checkOglErrors();
				}
				
// 				glClearColor(0.0, 0.0, 0.0, 1.0);
// 				glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
				
// 				glCullFace( GL_BACK );
// 				glEnable( GL_CULL_FACE );
// 				
// 				glEnable( GL_DEPTH_TEST );
				
// 				glPointSize( 2 );
// 				
// 				m_phong.render( m_mesh, *camera, light_trackball );
				
				camera->renderAtCorner();
			}
			
		private:
			SplatRenderer* m_renderer;
// 			Mesh m_mesh;
// 			Effects::Phong m_phong;
			SurfelVector m_surfels;
		};
		
        class SplatRendererTest : public ::testing::Test
		{
		protected:
			void SetUp() {}
		};
		
		TEST_F( SplatRendererTest, All )
		{
			{
				SplatRendererTestWidget widget;
				widget.show();
				widget.initialize();
				widget.resize( 640, 480 );

				QApplication::exec();
			}
			ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
		}
	}
}