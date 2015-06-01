#include "Point.h"

#include <gtest/gtest.h>
#include <iostream>

using namespace std;

namespace model
{
	namespace test
	{
        class PointTest : public ::testing::Test
		{
		protected:
			void SetUp() {}
		};

		TEST_F( PointTest, Add_operation )
		{
			auto point0 = Point( vec3( 1.f, 2.f ,3.f ), vec3( 4.f, 5.f, 6.f ) );
			auto point1 = Point( vec3( 10.f, 11.f ,12.f ), vec3( 13.f, 14.f, 15.f ) );
			auto point2 = Point( vec3( 20.f, 21.f ,21.f ), vec3( 23.f, 24.f, 25.f ) );
			Point calc0;
			Point calc1;
			Point expected;
			
			calc0 = point0 + point1;
			calc1 = point1 + point0;
			
			float epsilon = 1.e-15;
			
			expected = Point( vec3( 11.f, 13.f, 15.f ), vec3( 17.f, 19.f, 21.f ) );
			ASSERT_TRUE( calc0.equal( expected, epsilon ) );
			ASSERT_TRUE( calc0.equal( calc1, epsilon) );
			
			calc0 = ( point0 + point1 ) + point2;
			expected = Point( vec3( 31.f, 34.f, 36.f ), vec3( 40.f, 43.f, 46.f ) );
			ASSERT_TRUE( calc0.equal( expected, epsilon ) );
			
			calc0 = point0 + ( point1 + point2 );
			expected = Point( vec3( 31.f, 34.f, 36.f ), vec3( 40.f, 43.f, 46.f ) );
			ASSERT_TRUE( calc0.equal( expected, epsilon ) );
		}
		
		TEST_F( PointTest, Multiply_operation ) {
			auto point = Point( vec3( 1.f, 2.f ,3.f ), vec3( 4.f, 5.f, 6.f ) );
			float multiplier = 2.f;
			Point calc = point.multiply( multiplier );
			Point expected( vec3( 2.f, 4.f, 6.f ), vec3( 8.f, 10.f, 12.f ) );
			
			float epsilon = 1.e-15;
			ASSERT_TRUE( calc.equal( expected, epsilon ) );
		}
		
		/*TEST_F( PointTest, CopyConstructorAndAssignmentOp )
		{
			Point p0( vec3( 1.f, 2.f, 3.f ), vec3( 1.f, 2.f, 3.f ) );
			Point p1( p0 );
			p0 = p0 + p1;
			
			float epsilon = 1.e-15;
			ASSERT_TRUE( p1.equal( Point( vec3( 1.f, 2.f, 3.f ), vec3( 1.f, 2.f, 3.f ) ), epsilon ) );
			ASSERT_TRUE( p0.equal( Point( vec3( 2.f, 4.f, 6.f ), vec3( 2.f, 4.f, 6.f ) ), epsilon ) );
			
			p0 = p1;
			p1 = p0 + p1;
			ASSERT_TRUE( p0.equal( Point( vec3( 1.f, 2.f, 3.f ), vec3( 1.f, 2.f, 3.f ) ), epsilon ) );
			ASSERT_TRUE( p1.equal( Point( vec3( 2.f, 4.f, 6.f ), vec3( 2.f, 4.f, 6.f ) ), epsilon ) );
		}*/
	}
}