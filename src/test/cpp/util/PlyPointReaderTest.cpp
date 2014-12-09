#include "PlyPointReader.h"
#include "Stream.h"
#include <gtest/gtest.h>
#include <iostream>
#include <QApplication>

using namespace std;

extern "C" int g_argc;
extern "C" char** g_argv;

namespace util
{
	namespace test
	{
        class PlyPointReaderTest : public ::testing::Test
		{
		protected:
			static void SetUpTestCase() {
				QApplication app( g_argc, g_argv );
				m_appDirPath = new string( QCoreApplication::applicationDirPath().toStdString() );
			}
			
			static void TearDownTestCase() {
				delete m_appDirPath;
				m_appDirPath = nullptr;
			}
			
			static string* m_appDirPath;
		};
		
		string* PlyPointReaderTest::m_appDirPath = NULL;

		TEST_F( PlyPointReaderTest, Read )
		{
			SimplePointReader reader( *m_appDirPath + "/../../../src/data/tests/test.ply",
									  SimplePointReader::SINGLE, COLORS );
			PointVector< float, vec3 > points = reader.getPoints();
			
			Point< float, vec3 > expectedPoint0( vec3( ( float )81 / 255, ( float )63 / 255, ( float )39 / 255 ),
												 vec3( 7.163479, 4.658535, 11.321565 ) );
			Point< float, vec3 > expectedPoint1( vec3( ( float )146 / 255, ( float )142 / 255, ( float )143 / 255),
												 vec3( 6.996898, 5.635769, 11.201763 ) );
			Point< float, vec3 > expectedPoint2( vec3( ( float )128 / 255, ( float )106 / 255, ( float )82 / 255),
												 vec3( 7.202037, 4.750132, 11.198129 ) );
			
			float epsilon = 1.e-15;
			
			ASSERT_TRUE( reader.getAttributes() == model::COLORS );
			ASSERT_TRUE( expectedPoint0.equal( *points[0], epsilon ) );
			ASSERT_TRUE( expectedPoint1.equal( *points[1], epsilon ) );
			ASSERT_TRUE( expectedPoint2.equal( *points[2], epsilon ) );
		}
		
		TEST_F( PlyPointReaderTest, ReadNormals )
		{
			
			SimplePointReader reader( *m_appDirPath + "/../../../src/data/tests/test_normals.ply",
									  SimplePointReader::SINGLE, NORMALS );
			PointVector< float, vec3 > points = reader.getPoints();
			
			Point< float, vec3 > expectedPoint0( vec3( 11.321565, 4.658535, 7.163479 ),
												 vec3( 7.163479, 4.658535, 11.321565 ) );
			Point< float, vec3 > expectedPoint1( vec3( 11.201763, 5.635769, 6.996898 ),
												 vec3( 6.996898, 5.635769, 11.201763 ) );
			Point< float, vec3 > expectedPoint2( vec3( 11.198129, 4.750132, 7.202037 ),
												 vec3( 7.202037, 4.750132, 11.198129 ) );
			
			float epsilon = 1.e-15;
			
			ASSERT_TRUE( reader.getAttributes() == model::NORMALS );
			ASSERT_TRUE( expectedPoint0.equal( *points[0], epsilon ) );
			ASSERT_TRUE( expectedPoint1.equal( *points[1], epsilon ) );
			ASSERT_TRUE( expectedPoint2.equal( *points[2], epsilon ) );
		}
		
		TEST_F( PlyPointReaderTest, ReadExtendedPoints )
		{
			ExtendedPointReader reader( *m_appDirPath + "/../../../src/data/tests/test_extended_points.ply",
										ExtendedPointReader::SINGLE, COLORS_AND_NORMALS );
			ExtendedPointVector< float, vec3 > points = reader.getPoints();
			
			ExtendedPoint< float, vec3 > expectedPoint0( vec3( 0.003921569, 0.007843137, 0.011764706 ),
														 vec3( 11.321565, 4.658535, 7.163479 ),
														 vec3( 7.163479, 4.658535, 11.321565 ) );
			ExtendedPoint< float, vec3 > expectedPoint1( vec3( 0.015686275, 0.019607843, 0.023529412 ),
														 vec3( 11.201763, 5.635769, 6.996898 ),
														 vec3( 6.996898, 5.635769, 11.201763 ) );
			ExtendedPoint< float, vec3 > expectedPoint2( vec3( 0.02745098, 0.031372549, 0.035294118 ),
														 vec3( 11.198129, 4.750132, 7.202037 ),
														 vec3( 7.202037, 4.750132, 11.198129 ) );
			
			ASSERT_TRUE( reader.getAttributes() == model::COLORS_AND_NORMALS );
			
			float epsilon = 1.e-15;
			
			ASSERT_TRUE( expectedPoint0.equal( *points[0], epsilon ) );
			ASSERT_TRUE( expectedPoint1.equal( *points[1], epsilon ) );
			ASSERT_TRUE( expectedPoint2.equal( *points[2], epsilon ) );
		}
	}
}