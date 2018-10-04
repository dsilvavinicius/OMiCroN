#include <gtest/gtest.h>
#include <utils/qtfreecamerawidget.hpp>
#include <QApplication>
#include "omicron/disk/ply_point_reader.h"
#include "omicron/renderer/splat_renderer/splat_renderer.hpp"
// #include "splat_renderer/surfel.hpp"

#include "omicron/renderer/ogl_utils.h"
#include "omicron/disk/ply_point_writter.h"
#include "tucano/effects/phongshader.hpp"
//#include "omicron/hierarchy/node_loader.h"
#include "omicron/ui/gl_hidden_widget.h"

#define USE_SPLAT
#define N_SEGMENTS 4

namespace omicron::test
{
    using namespace std;
    using namespace Tucano;
    using namespace util;
    using namespace renderer;
    
    using SurfelArray = Array< Surfel >;
    using Node = O1OctreeNode< Surfel >;
    using Siblings = Array< Node >;
    using NodeLoader = hierarchy::NodeLoader< Surfel >;
    
    class SplatRendererTestWidget
    : public QtFreecameraWidget
    {
    public:
        #ifdef USE_SPLAT
            SplatRendererTestWidget( NodeLoader& loader, QWidget *parent = 0 )
            : QtFreecameraWidget( parent, loader.widget() ),
            m_loader( loader ),
            m_siblings( N_SEGMENTS ),
            m_surfels( N_SEGMENTS ),
            m_renderer( nullptr )
        #else
            SplatRendererTestWidget( QWidget *parent = 0 )
            : QtFreecameraWidget( parent )
        #endif
        
        {}
        
        ~SplatRendererTestWidget()
        {
            #ifdef USE_SPLAT
                delete m_renderer;
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
                int nSurfels = reader.getNumPoints();
                int surfelsPerSegment = ceil( float( nSurfels ) / N_SEGMENTS );
                
                for( int segment = 0; nSurfels > 0; nSurfels -= surfelsPerSegment )
                {
                    int segmentSurfels = ( nSurfels - surfelsPerSegment >= 0 ) ?
                        surfelsPerSegment : nSurfels - surfelsPerSegment;
                    
                    m_surfels[ segment++ ] = SurfelArray( segmentSurfels );
                }
            #else
                vector< Vector4f > vertices;
                vector< Vector3f > normals;
            #endif
            
            int pointIdx = 0;
                
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
                        
                        int segmentIdx = pointIdx / surfelsPerSegment;
                        int surfelIdx = pointIdx++ % surfelsPerSegment;
                        m_surfels[ segmentIdx ][ surfelIdx ] = s;
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
                for( SurfelArray& surfelArray : m_surfels )
                {
                    for( Surfel& surfel : surfelArray )
                    {
                        surfel.c = ( surfel.c - midPoint ) * scale;
                    }
                }
// 					Matrix4f transform = Translation3f( -midPoint * scale ) * Scaling( scale ).matrix();
                Matrix4f transform = Matrix4f::Identity();
            
                m_renderer = new SplatRenderer( camera );
                
                for( int i = 0; i < N_SEGMENTS; ++i )
                {
                    m_siblings[ i ] = Node( m_surfels[ i ], true );
// 						m_siblings[ i ].loadGPU();
                    m_loader.asyncLoad( m_siblings[ i ], 0 );
                    
                    OglUtils::checkOglErrors();
                }
                m_loader.onIterationEnd();
                
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
            glClearColor( 1.0f, 1.0f, 1.0f, 1.0f ); 
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            #ifdef USE_SPLAT
                m_renderer->begin_frame();
            
                if( !m_siblings.empty() )
                {
                    for( Node& node : m_siblings )
                    {
                        if( node.isLoaded() )
                        {
                            m_renderer->render( node );
                        }
                    }
                }
            #else
                Effects::Phong phong;
                phong.setShadersDir( "../shaders/tucano/" );
                phong.initialize();
                phong.render( m_mesh, *camera, light_trackball );
            #endif
                
                OglUtils::checkOglErrors();
                        
            #ifdef USE_SPLAT
                m_renderer->render_frame();
                m_loader.onIterationEnd();
            #endif
            
            camera->renderAtCorner();
        }
        
        #ifdef USE_SPLAT
            void keyPressEvent( QKeyEvent * event ) override
            {
                QtFlycameraWidget::keyPressEvent( event );
                
                switch( event->key() )
                {
                    case Qt::Key_F1 :
                    {
                        if( !m_siblings.empty() )
                        {
                            for( Node& sibling : m_siblings )
                            {
                                m_loader.asyncLoad( sibling, 0 );
                            }
                        }
                        break;
                    }
                    case Qt::Key_F2 :
                    {
                        if( !m_siblings.empty() )
                        {
                            for( Node& sibling : m_siblings )
                            {
                                m_loader.asyncUnload( sibling, 0 );
                            }
                        }
                        break;
                    }
                    case Qt::Key_F3 :
                    {
                        if( m_siblings.empty() )
                        {
                            m_siblings = Siblings( N_SEGMENTS );
                            
                            for( int i = 0; i < N_SEGMENTS; ++i )
                            {
                                m_siblings[ i ] = Node( m_surfels[ i ], true );
                                m_loader.asyncLoad( m_siblings[ i ], 0 );
                            }
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
        #endif
        
    private:
        #ifdef USE_SPLAT
            SplatRenderer* m_renderer;
            NodeLoader& m_loader;
            Array< SurfelArray > m_surfels;
            Siblings m_siblings;
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
            #ifdef USE_SPLAT
                GLHiddenWidget hiddenWidget;
                NodeLoaderThread loaderThread( &hiddenWidget, 900ul * 1024ul * 1024ul );
                NodeLoader loader( &loaderThread, 1 );
            #endif
        
            {
                SplatRendererTestWidget widget(
                    #ifdef USE_SPLAT
                        loader
                    #endif
                );
                widget.initialize();
                widget.show();
                widget.resize( 640, 480 );

                QApplication::exec();
            }
            
            #ifdef USE_SPLAT
                ASSERT_EQ( 0, loader.memoryUsage() );
            #endif
        }
        ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
    }
}
