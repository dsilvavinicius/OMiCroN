#include "PlyPointReader.h"
#include "Stream.h"
#include <gtest/gtest.h>
#include <iostream>

using namespace std;

namespace util
{
	namespace test
	{
        class PlyPointReaderTest : public ::testing::Test
		{
		protected:
			void SetUp() {}
		};

		TEST_F( PlyPointReaderTest, Read )
		{
			PlyPointReader< float, vec3 > reader( "../../../src/data/tests/test.ply", PlyPointReader< float, vec3 >::SINGLE );
			PointVector< float, vec3 > points = reader.getPoints();
			
			Point< float, vec3 > expectedPoint0( vec3( ( float )81 / 255, ( float )63 / 255, ( float )39 / 255 ),
												 vec3( 7.163479, 4.658535, 11.321565 ) );
			Point< float, vec3 > expectedPoint1( vec3( ( float )146 / 255, ( float )142 / 255, ( float )143 / 255),
												 vec3( 6.996898, 5.635769, 11.201763 ) );
			Point< float, vec3 > expectedPoint2( vec3( ( float )128 / 255, ( float )106 / 255, ( float )82 / 255),
												 vec3( 7.202037, 4.750132, 11.198129 ) );
			
			ASSERT_FALSE( reader.hasNormals() );
			ASSERT_TRUE( expectedPoint0.equal( *points[0] ) );
			ASSERT_TRUE( expectedPoint1.equal( *points[1] ) );
			ASSERT_TRUE( expectedPoint2.equal( *points[2] ) );
		}
		
		TEST_F( PlyPointReaderTest, ReadNormals )
		{
			PlyPointReader< float, vec3 > reader( "../../../src/data/tests/test_normals.ply",
												  PlyPointReader< float, vec3 >::SINGLE );
			PointVector< float, vec3 > points = reader.getPoints();
			
			Point< float, vec3 > expectedPoint0( vec3( 11.321565, 4.658535, 7.163479 ),
												 vec3( 7.163479, 4.658535, 11.321565 ) );
			Point< float, vec3 > expectedPoint1( vec3( 11.201763, 5.635769, 6.996898 ),
												 vec3( 6.996898, 5.635769, 11.201763 ) );
			Point< float, vec3 > expectedPoint2( vec3( 11.198129, 4.750132, 7.202037 ),
												 vec3( 7.202037, 4.750132, 11.198129 ) );
			
			ASSERT_TRUE( reader.hasNormals() );
			ASSERT_TRUE( expectedPoint0.equal( *points[0] ) );
			ASSERT_TRUE( expectedPoint1.equal( *points[1] ) );
			ASSERT_TRUE( expectedPoint2.equal( *points[2] ) );
		}
	}
}