#include <gtest/gtest.h>
#include <iostream>
#include <QApplication>
#include "PlyPointReader.h"
#include "Stream.h"
#include "MortonCode.h"
#include "OctreeNode.h"
#include "MemoryManagerTypes.h"

using namespace std;

namespace util
{
	namespace test
	{
        class PlyPointReaderTest : public ::testing::Test
		{};

		TEST_F( PlyPointReaderTest, ReadNormals )
		{
			using PointPtr = shared_ptr< Point >;
			using PointVector = vector< PointPtr, ManagedAllocator< PointPtr > >;
			
			SPV_DefaultManager::initInstance( 1000000 );
			
			PointVector points;
			SimplePointReader reader( "data/test_normals.ply" );
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
		
		TEST_F( PlyPointReaderTest, ReadExtendedPoints )
		{
			using Point = model::ExtendedPoint;
			using PointPtr = shared_ptr< Point >;
			using PointVector = vector< PointPtr, ManagedAllocator< PointPtr > >;
			
			SEV_DefaultManager::initInstance( 1000000 );
			
			PointVector points;
			ExtendedPointReader reader( "data/test_extended_points.ply" );
			reader.read( 
				[ & ]( const Point& point ){ points.push_back( makeManaged< Point >( point ) ); }
			);
			
			ASSERT_EQ( 3, points.size() );
			
			Point expectedPoint0( Vec3( 0.003921569, 0.007843137, 0.011764706 ), Vec3( 11.321565, 4.658535, 7.163479 ),
								  Vec3( 7.163479, 4.658535, 11.321565 ) );
			Point expectedPoint1( Vec3( 0.015686275, 0.019607843, 0.023529412 ), Vec3( 11.201763, 5.635769, 6.996898 ),
								  Vec3( 6.996898, 5.635769, 11.201763 ) );
			Point expectedPoint2( Vec3( 0.02745098, 0.031372549, 0.035294118 ), Vec3( 11.198129, 4.750132, 7.202037 ),
								  Vec3( 7.202037, 4.750132, 11.198129 ) );
			
			float epsilon = 1.e-6;
			
			// Debug
			{
				cout << *points[ 0 ] << endl;
			}
			
			ASSERT_TRUE( expectedPoint0.equal( *points[ 0 ], epsilon ) );
			ASSERT_TRUE( expectedPoint1.equal( *points[ 1 ], epsilon ) );
			ASSERT_TRUE( expectedPoint2.equal( *points[ 2 ], epsilon ) );
		}
	}
}