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

#define USE_SPLAT

namespace model
{
	namespace test
	{
		class SplatRendererTestWidget
		: public QtFreecameraWidget
		{
		public:
			SplatRendererTestWidget( QWidget *parent = 0 )
			: QtFreecameraWidget( parent )
			
			#ifdef USE_SPLAT
				, m_renderer( nullptr )
			#endif
			
			{}
			
			~SplatRendererTestWidget()
			{
				#ifdef USE_SPLAT
					delete m_renderer;
					delete m_cloud;
				#endif
			}
			
			void initialize()
			{
				QtFreecameraWidget::initialize();
				
				PlyPointReader reader( "../data/example/staypuff.ply" );
				
				Float negInf = -numeric_limits< Float >::max();
				Float posInf = numeric_limits< Float >::max();
				Vec3 origin = Vec3( posInf, posInf, posInf );
				Vec3 maxCoords( negInf, negInf, negInf );
			
				#ifdef USE_SPLAT
					Array< Surfel > surfels( reader.getNumPoints() );
					auto surfelIter = surfels.begin();
				#else
					vector< Vector4f > vertices;
					vector< Vector3f > normals;
				#endif
				
				reader.read(
					[ & ]( const Point& p )
					{
						const Vec3& pos = p.getPos();
						const Vector3f& normal = p.getNormal();
						
						#ifdef USE_SPLAT
							Vector3f pointOnPlane(
								( normal.x() * pos.x() + normal.y() * pos.y() + normal.z() * pos.z() ) / normal.x(),
								0.f, 0.f );
							Vector3f u = pointOnPlane - pos;
							u.normalize();
							Vector3f v = normal.cross( u );
			
							u *= 0.003f;
							v *= 0.003f;
						
							Surfel s( pos, u, v );
							*surfelIter++ = s;
						#else
							vertices.push_back( Vector4f( pos.x(), pos.y(), pos.z(), 1.f ) );
							normals.push_back( normal );
						#endif
						
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
				
				#ifdef USE_SPLAT
					for( Surfel& surfel : surfels )
					{
						surfel.c = ( surfel.c - midPoint ) * scale;
					}
// 					Matrix4f transform = Translation3f( -midPoint * scale ) * Scaling( scale ).matrix();
					Matrix4f transform = Matrix4f::Identity();
				
					m_renderer = new SplatRenderer( camera );
					m_cloud = new SurfelCloud( surfels, transform );
				#else
					Affine3f transform = Translation3f( -midPoint * scale ) * Scaling( scale );
					m_mesh.setModelMatrix( transform );
					
					m_mesh.loadVertices( vertices );
					m_mesh.loadNormals( normals );
				#endif
				
				OglUtils::checkOglErrors();
			}
			
			void resizeGL (int w, int h) override
			{
				QtFreecameraWidget::resizeGL( w, h );
				
				#ifdef USE_SPLAT
					if( m_renderer != nullptr )
					{
						m_renderer->reshape( w, h );
					}
				#endif
			}
			
			void paintGL() override
			{
				#ifdef USE_SPLAT
					m_renderer->begin_frame();
					m_renderer->render_cloud( *m_cloud );
					m_renderer->end_frame();
				#else
					glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
					
					Effects::Phong phong;
					phong.setShadersDir( "../shaders/tucano/" );
					phong.initialize();
					phong.render( m_mesh, *camera, light_trackball );
				#endif
				
				OglUtils::checkOglErrors();
				camera->renderAtCorner();
			}
			
		private:
			#ifdef USE_SPLAT
				SplatRenderer* m_renderer;
				SurfelCloud* m_cloud;
			#else
				Mesh m_mesh;
			#endif
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
				widget.initialize();
				widget.show();
				widget.resize( 640, 480 );

				QApplication::exec();
			}
			ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
		}
	}
}