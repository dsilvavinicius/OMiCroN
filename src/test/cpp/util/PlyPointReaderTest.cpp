#include <gtest/gtest.h>
#include <iostream>
#include <QApplication>
#include "PlyPointReader.h"
#include "Stream.h"
#include <Profiler.h>
#include "MortonCode.h"

using namespace std;

namespace util
{
	namespace test
	{
        class PlyPointReaderTest : public ::testing::Test
		{
			void SetUp()
			{
				setlocale( LC_NUMERIC, "C" );
			}
		};

		TEST_F( PlyPointReaderTest, ReadNormals )
		{
			PointVector points;
			PlyPointReader reader( "data/test_normals.ply" );
			reader.read(
				[ & ]( const Point& point ){ points.push_back( makeManaged< Point >( point ) ); }
			);
			
			ASSERT_EQ( 3, points.size() );
			
			Point expectedPoint0( Vec3( 11.321565, 4.658535, 7.163479 ), Vec3( 7.163479, 4.658535, 11.321565 ) );
			Point expectedPoint1( Vec3( 11.201763, 5.635769, 6.996898 ), Vec3( 6.996898, 5.635769, 11.201763 ) );
			Point expectedPoint2( Vec3( 11.198129, 4.750132, 7.202037 ), Vec3( 7.202037, 4.750132, 11.198129 ) );
			
			float epsilon = 1.e-15;
			
			ASSERT_TRUE( expectedPoint0.equal( *points[0], epsilon ) );
			ASSERT_TRUE( expectedPoint1.equal( *points[1], epsilon ) );
			ASSERT_TRUE( expectedPoint2.equal( *points[2], epsilon ) );
		}
		
		TEST_F( PlyPointReaderTest, ProfileDavidReading )
		{
			string taskName = "David reading";
			
			auto start = Profiler::now( taskName );
			
			Point dontCare;
			PlyPointReader reader( "/home/vinicius/Datasets/David/Sorted_13Lvls/David.ply" );
			reader.read(
				[ & ]( const Point& p )
				{
					dontCare = p;
				}
			);
			
			Profiler::elapsedTime( start, taskName );
		}
	}
}