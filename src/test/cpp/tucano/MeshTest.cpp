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
			{
				PlyPointReader< ExtendedPoint > reader( "data/example/staypuff.ply" );
				ulong nPoints = reader.getNumPoints();
				
				mesh.reset();
				mesh.reserveVertices( 3, nPoints );
				mesh.reserveNormals( nPoints );
				mesh.reserveColors( 3, nPoints );
				mesh.reserveIndices( nPoints );
				
				float* vertPtr = mesh.mapVertices( 0, nPoints );
				float* normalPtr = mesh.mapNormals( 0, nPoints );
				float* colorPtr = mesh.mapColors( 0, nPoints );
				
				ulong insertedPoints = 0;
				reader.read(
					[ & ]( const ExtendedPoint& p )
					{
						vertPtr[ insertedPoints * 3 ]		= p.getPos().x();
						vertPtr[ insertedPoints * 3 + 1 ]	= p.getPos().y();
						vertPtr[ insertedPoints * 3 + 2 ]	= p.getPos().z();
						
						normalPtr[ insertedPoints * 3 ]		= p.getNormal().x();
						normalPtr[ insertedPoints * 3 + 1 ]	= p.getNormal().y();
						normalPtr[ insertedPoints * 3 + 2 ]	= p.getNormal().z();
						
						colorPtr[ insertedPoints * 3 ]		= p.getColor().x();
						colorPtr[ insertedPoints * 3 + 1 ]	= p.getColor().y();
						colorPtr[ insertedPoints * 3 + 2 ]	= p.getColor().z();
						
						++insertedPoints;
					}
				);
				
				m_phong.initialize();
			}
			
			void paintGL() override
			{
				glClearColor(1.0, 1.0, 1.0, 0.0);
				glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		
				glCullFace( GL_BACK );
				glEnable( GL_CULL_FACE );
		
				glEnable( GL_DEPTH_TEST );
				
				m_phong.render( mesh, *camera, light_trackball );
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

			QApplication::exec();
		}
	}
}