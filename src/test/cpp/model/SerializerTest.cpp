#include <gtest/gtest.h>
#include "Serializer.h"

namespace model
{
	namespace test
	{
        class SerializerTest
        : public ::testing::Test
		{
		protected:
			void SetUp(){}
		};
		
		TEST_F( SerializerTest, PointVector )
		{
			Point p0( Vec3( 11.321565f, 4.658535f, 7.163479f ), Vec3( 7.163479f, 4.658535f, 11.321565f ) );
			Point p1( Vec3( 11.201763f, 5.635769f, 6.996898f ), Vec3( 6.996898f, 5.635769f, 11.201763f ) );
			Point p2( Vec3( 11.198129f, 4.750132f, 7.202037f ), Vec3( 7.202037f, 4.750132f, 11.198129f ) );
			
			PointPtr pointArray[3] = { makeManaged< Point >( p0 ), makeManaged< Point >( p1 ), makeManaged< Point >( p2 ) };
			PointVector points( pointArray, pointArray + 3 );
			
			byte* bytes;
			Serializer::serialize( points, &bytes );
			
			PointVector deserializedPoints;
			Serializer::deserialize( bytes, deserializedPoints );
			
			float epsilon = 1.e-15;
			for( int i = 0; i < points.size(); ++i )
			{
				ASSERT_TRUE( points[ i ]->equal( *deserializedPoints[ i ], epsilon ) );
			}
			
			Serializer::dispose( bytes );
		}
	}
}