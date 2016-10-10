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
		
		template< typename Point >
		void createArray( Array< shared_ptr< Point > >& array )
		{
			for( int i = 0; i < array.size(); ++i )
			{
				array[ i ] = makeManaged< Point >( createPoint< Point >( i ) );
			}
		}
		
		typedef Types< Point > PointTypes;
		
		TYPED_TEST_CASE( ArrayTest, PointTypes );
		
		template< typename Point >
		void testAccess( Array< shared_ptr< Point > >& array, uint expectedSize )
		{
			using PointPtr = shared_ptr< Point >;
			using PointArray = Array< PointPtr >;
			
			ASSERT_EQ( expectedSize, array.size() );
			
			for( int i = 0; i < array.size(); ++i )
			{
				Point expected = createPoint< Point >( i );
				
// 				cout << *array[ i ] << endl << "Expected: " << expected << endl;
				
				ASSERT_TRUE( expected.equal( *array[ i ], 1.e-15 ) );
			}
			
			int i = 0;
			for( PointPtr& point : array )
			{
				Point expected = createPoint< Point >( i++ );
				ASSERT_TRUE( expected.equal( *point, 1.e-15 ) );
			}
			
			i = 0;
			for( const PointPtr& point : array )
			{
				Point expected = createPoint< Point >( i++ );
				ASSERT_TRUE( expected.equal( *point, 1.e-15 ) );
			}
			
			size_t expectedSerialSize = sizeof( uint ) + array.size() * sizeof( Point );
			byte* bytes = Serializer::newByteArray( expectedSerialSize );
			
			ASSERT_EQ( expectedSerialSize, array.serialize( &bytes ) );
			
			PointArray deserialized = PointArray::deserialize( bytes );
			
			for( int i = 0; i < array.size(); ++i )
			{
// 				cout << "Expected: " << *array[ i ] << endl
// 						<< "Got: " << *deserialized[ i ] << endl;
				ASSERT_TRUE( array[ i ]->equal( *deserialized[ i ], 1.e-15 ) );
			}
			
			Serializer::dispose( bytes );
		}
		
		TYPED_TEST( ArrayTest, API )
		{
			using Point = TypeParam;
			using PointPtr = shared_ptr< Point >;
			using PointArray = Array< PointPtr >;
			
			ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
			
			{
				int arraySize = 10;
				
				{
					SCOPED_TRACE( "Test0" );
					
					PointArray array;
					ASSERT_EQ( 0, array.size() );
					
					for( PointPtr& point : array )
					{
						ADD_FAILURE();
					}
				}
				{
					SCOPED_TRACE( "Test1" );
					
					PointArray array( arraySize );
					createArray( array );
					testAccess( array, arraySize );
				}
				{
					SCOPED_TRACE( "Test2" );
					
					PointArray array( arraySize );
					createArray( array );
					testAccess( array, arraySize );
					
					PointArray otherArray( array );
					testAccess( otherArray, arraySize );
				}
				{
					SCOPED_TRACE( "Test3" );
					
					PointArray array( arraySize );
					createArray( array );
					
					PointArray otherArray( arraySize );
					otherArray = array;
					
					testAccess( array, arraySize );
					testAccess( otherArray, arraySize );
				}
				{
					SCOPED_TRACE( "Test4" );
					
					TbbAllocator< PointPtr > alloc;
					PointPtr* rawArray = alloc.allocate( arraySize );
					for( int i = 0; i < arraySize; ++i )
					{
						alloc.construct( rawArray + i );
					}
					
					PointArray array( arraySize, rawArray );
					createArray( array );
					
					testAccess( array, arraySize );
				}
				{
					SCOPED_TRACE( "Test5" );
					
					Array< uint > array( arraySize, uint( 0 ) );
					for( uint& element : array )
					{
						ASSERT_EQ( 0, element );
					}
				}
				{
					SCOPED_TRACE( "Test6" );
					
					PointArray array( arraySize );
					createArray( array );
					
					PointArray otherArray( std::move( array ) );
					testAccess( otherArray, arraySize );
					
					ASSERT_TRUE( array.empty() );
				}
				{
					SCOPED_TRACE( "Test7" );
					
					PointArray array( arraySize );
					createArray( array );
					
					PointArray otherArray( arraySize );
					otherArray = std::move( array );
					testAccess( otherArray, arraySize );
					
					ASSERT_TRUE( array.empty() );
				}
			}
			
			ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
		}
	}
}