#include <gtest/gtest.h>
#include <iostream>

#include "Array.h"

using namespace std;

namespace model
{
	namespace test
	{
		template< typename T >
        class ArrayTest
        : public ::testing::Test
		{
		protected:
			void SetUp() {}
		};

		using testing::Types;
		
		template< typename P >
		P createPoint( float value )
		{
			P point;
			byte* tempPtr = reinterpret_cast< byte* >( &point );
			Vec3 vec( value, value, value );
			size_t vec3Size = sizeof( Vec3 );
			
			for( int j = 0; j < sizeof( P ) / vec3Size; ++j )
			{
				memcpy( tempPtr, &vec, vec3Size );
				tempPtr += vec3Size;
			}
			
			return point;
		}
		
		typedef Types< Point, ExtendedPoint > PointTypes;
		
		TYPED_TEST_CASE( ArrayTest, PointTypes );
		
		TYPED_TEST( ArrayTest, API )
		{
			using Point = TypeParam;
			using PointPtr = shared_ptr< Point >;
			using PointArray = Array< PointPtr >;
			
			ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
			
			{
				PointArray array;
				ASSERT_EQ( 0, array.size() );
				
				int arraySize = 10;
				array = PointArray( arraySize );
				
				for( int i = 0; i < arraySize; ++i )
				{
					array[ i ] = makeManaged< Point >( createPoint< Point >( i ) );
				}
				
				for( int i = 0; i < arraySize; ++i )
				{
					Point expected = createPoint< Point >( i );
					ASSERT_TRUE( expected.equal( *array[ i ], 1.e-15 ) );
				}
				
				size_t expectedSerialSize = sizeof( uint ) + array.size() * sizeof( Point );
				byte* bytes = Serializer::newByteArray( expectedSerialSize );
				
				ASSERT_EQ( expectedSerialSize, array.serialize( &bytes ) );
				
				PointArray deserialized = PointArray::deserialize( bytes );
				
				for( int i = 0; i < array.size(); ++i )
				{
					cout << "Expected: " << *array[ i ] << endl
						 << "Got: " << *deserialized[ i ] << endl;
					
					ASSERT_TRUE( array[ i ]->equal( *deserialized[ i ], 1.e-15 ) );
				}
				
				Serializer::dispose( bytes );
			}
			
			ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
		}
	}
}