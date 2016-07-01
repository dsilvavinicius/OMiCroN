#include <gtest/gtest.h>
#include <phongshader.hpp>
#include <utils/qtflycamerawidget.hpp>
#include <QApplication>
#include "PlyPointReader.h"

using namespace std;
using namespace Tucano;
using namespace Effects;
using namespace util;

namespace Tucano
{
	namespace test
	{
		class MeshTestWidget
		: public QtFlycameraWidget
		{
		public:
			MeshTestWidget( QWidget *parent = 0 )
			: QtFlycameraWidget( parent )
			{}
			
			void checkOglErrors()
			{
				GLenum err = GL_NO_ERROR;
				bool hasErrors = false;
				stringstream ss;
				while( ( err = glGetError() ) != GL_NO_ERROR )
				{
					hasErrors = true;
					ss  << "OpenGL error 0x" << hex << err << ": " << gluErrorString( err ) << endl << endl;
				}
				
				if( hasErrors )
				{
					throw runtime_error( ss.str() );
				}
			}
			
			
			
			void initialize()
			{
				QtFlycameraWidget::initialize();
				
				PlyPointReader< ExtendedPoint > reader( "../data/example/staypuff.ply" );
				ulong nPoints = reader.getNumPoints();
				
				mesh.reset();
				mesh.reserveVertices( 3, nPoints );
				checkOglErrors();
				
				mesh.reserveNormals( nPoints );
				checkOglErrors();
				
				mesh.reserveColors( 3, nPoints );
				checkOglErrors();
				
				mesh.reserveIndices( nPoints );
				checkOglErrors();
				
				float* vertsPtr = mesh.mapVertices( 0, nPoints );
				checkOglErrors();
				
				float* normalsPtr = mesh.mapNormals( 0, nPoints );
				checkOglErrors();
				
				float* colorsPtr = mesh.mapColors( 0, nPoints );
				checkOglErrors();
				
				uint* indicesPtr = mesh.mapIndices( 0, nPoints );
				checkOglErrors();
				
				ulong insertedPoints = 0;
				
				Float negInf = -numeric_limits< Float >::max();
				Float posInf = numeric_limits< Float >::max();
				Vec3 origin = Vec3( posInf, posInf, posInf );
				Vec3 maxCoords( negInf, negInf, negInf );
				
				reader.read(
					[ & ]( const ExtendedPoint& p )
					{
						const Vec3& pos = p.getPos();
						
						vertsPtr[ insertedPoints * 3 ]		= pos.x();
						vertsPtr[ insertedPoints * 3 + 1 ]	= pos.y();
						vertsPtr[ insertedPoints * 3 + 2 ]	= pos.z();
						
						normalsPtr[ insertedPoints * 3 ]		= p.getNormal().x();
						normalsPtr[ insertedPoints * 3 + 1 ]	= p.getNormal().y();
						normalsPtr[ insertedPoints * 3 + 2 ]	= p.getNormal().z();
						
						colorsPtr[ insertedPoints * 3 ]		= p.getColor().x();
						colorsPtr[ insertedPoints * 3 + 1 ]	= p.getColor().y();
						colorsPtr[ insertedPoints * 3 + 2 ]	= p.getColor().z();
						
						indicesPtr[ insertedPoints ] = insertedPoints;
						
						for( int i = 0; i < 3; ++i )
						{
							origin[ i ] = std::min( origin[ i ], pos[ i ] );
							maxCoords[ i ] = std::max( maxCoords[ i ], pos[ i ] );
						}
						
						++insertedPoints;
					}
				);
				
				Vec3 boxSize = maxCoords - origin;
				float scale = 1.f / std::max( std::max( boxSize.x(), boxSize.y() ), boxSize.z() );
				
				for( int i = 0; i < insertedPoints; ++i )
				{
					vertsPtr[ insertedPoints * 3 ]		= ( vertsPtr[ insertedPoints * 3 ] - origin.x() ) * scale;
					vertsPtr[ insertedPoints * 3 + 1 ]	= ( vertsPtr[ insertedPoints * 3 + 1 ] - origin.y() ) * scale;
					vertsPtr[ insertedPoints * 3 + 2 ]	= ( vertsPtr[ insertedPoints * 3 + 2 ] - origin.z() ) * scale;
				}
				
				mesh.unmapVertices();
				checkOglErrors();
				
				mesh.unmapNormals();
				checkOglErrors();
				
				mesh.unmapColors();
				checkOglErrors();
				
				mesh.unmapIndices();
				checkOglErrors();
				
				mesh.selectPrimitive( Mesh::POINT );
				
				m_phong.setShadersDir( "../shaders/tucano/" );
				m_phong.initialize();
			}
			
			void paintGL() override
			{
				glClearColor(1.0, 1.0, 1.0, 0.0);
				glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		
				glCullFace( GL_BACK );
				glEnable( GL_CULL_FACE );
		
				glEnable( GL_DEPTH_TEST );
				
				checkOglErrors();
				
				m_phong.render( mesh, *camera, light_trackball );
				
				checkOglErrors();
				
				camera->renderAtCorner();
				
				checkOglErrors();
			}
		
		private:
			Phong m_phong;
		};
		
        class MeshTest : public ::testing::Test
		{
		protected:
			void SetUp() {}
		};
		
		TEST_F( MeshTest, Loading )
		{
			MeshTestWidget widget;
			widget.resize( 640, 480 );
			widget.show();
			widget.initialize();

			QApplication::exec();
		}
	}
}