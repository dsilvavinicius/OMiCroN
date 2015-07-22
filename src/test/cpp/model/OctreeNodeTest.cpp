#include <gtest/gtest.h>
#include <vector>
#include "Point.h"
#include "OctreeNode.h"

using namespace std;

namespace model
{
	namespace test
	{
        class OctreeNodeTest : public ::testing::Test
		{
		protected:
			void SetUp() {}
		};

		TEST_F( OctreeNodeTest, CreationAndDestruction )
		{
			Point p0( vec3( 1.f, 2.f, 3.f ), vec3( 4.f, 5.f, 6.f ) );
			Point p1( vec3( 7.f, 8.f, 9.f ), vec3( 10.f, 11.f, 12.f ) );
			PointVector points;
			points.push_back( PointPtr( new Point( p0 ) ) );
			points.push_back( PointPtr( new Point( p1 ) ) );
			
			ShallowLeafNodePtr< PointVector > node( new ShallowLeafNode< PointVector >() );
			node->setContents( points );
			
			float epsilon = 1.e-15f;
			
			const PointVector& pointsGot = node->getContents();
			ASSERT_TRUE( pointsGot[ 0 ]->equal( p0, epsilon ) );
			ASSERT_TRUE( pointsGot[ 1 ]->equal( p1, epsilon ) );
			ASSERT_EQ( pointsGot[ 0 ].use_count(), 2 );
			ASSERT_EQ( pointsGot[ 1 ].use_count(), 2 );
			ASSERT_EQ( points[ 0 ].use_count(), 2 );
			ASSERT_EQ( points[ 1 ].use_count(), 2 );
			
			node = nullptr;
			ASSERT_EQ( points[ 0 ].use_count(), 1 );
			ASSERT_EQ( points[ 1 ].use_count(), 1 );
			ASSERT_TRUE( points[ 0 ]->equal( p0, epsilon ) );
			ASSERT_TRUE( points[ 1 ]->equal( p1, epsilon ) );
		}
	}
}