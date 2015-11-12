#include "TbbAllocator.h"
#include <Point.h>

#include <gtest/gtest.h>
#include <iostream>
#include <omp.h>

using namespace std;

namespace model
{
	namespace test
	{
		template< typename T >
        class TbbAllocatorTest : public ::testing::Test
		{
		};
		
		struct ArrayTypes
		{
			using PointVector = Point*;
			using ThreadVector = PointVector*;
			using TestVector = ThreadVector*;
		};
		
		struct VectorTypes
		{
			using PointVector = vector< Point, TbbAllocator< Point > >;
			using ThreadVector = vector< PointVector, TbbAllocator< PointVector > >;
			using TestVector = vector< ThreadVector, TbbAllocator< ThreadVector > >;
		};
		
		template< typename T, typename A >
		void initVector( vector< T, A >& vec, const size_t& size )
		{
			vec.resize( size );
		}
		
		template< typename T >
		void initVector( T*& vec, const size_t& size )
		{
			vec = TbbAllocator< T >().allocate( size );
		}
		
		template< typename T, typename A >
		void releaseVector( vector< T, A >& vec )
		{}
		
		template< typename T >
		void releaseVector( T*& vec )
		{
			TbbAllocator< T >().deallocate( vec );
		}
		
		using testing::Types;
		
		typedef Types< ArrayTypes, VectorTypes > Implementations;
		
		TYPED_TEST_CASE( TbbAllocatorTest, Implementations );
		
		TYPED_TEST( TbbAllocatorTest, Allocation )
		{
			using PointVector = typename TypeParam::PointVector;
			using ThreadVector = typename TypeParam::ThreadVector;
			using TestVector = typename TypeParam::TestVector;
			
			int nThreads = 8;
			
			{
				TestVector vectors; initVector( vectors, nThreads );
			
				int vectorsPerThread = 10000;
				int pointsPerIter = 3500;
				#pragma omp parallel for
				for( int i = 0; i < nThreads; i++ )
				{
					ThreadVector threadVector; initVector( threadVector, vectorsPerThread );
					PointVector points; initVector( points, pointsPerIter );
					
					int j = 0;
					while( j < vectorsPerThread )
					{
						threadVector[ j ] = points;
						++j;
					}
					
					vectors[ i ] = threadVector;
				}
				
				size_t expected = 	nThreads * (
										sizeof( ThreadVector )
										+ vectorsPerThread * (
											sizeof( PointVector ) + pointsPerIter * sizeof( Point )
										)
									);
				size_t allocated = AllocStatistics::totalAllocated();
				cout << "Expected: " << expected << ", Allocated: " << allocated << ", overhead: "
					 << float( allocated - expected ) / expected << endl << endl;
					 
				#pragma omp parallel for
				for( int i = 0; i < nThreads; i++ )
				{
					int j = 0;
					while( j < vectorsPerThread )
					{
						releaseVector( vectors[ i ][ j ] );
					}
					
					releaseVector( vectors[ i ] );
				}
				
				releaseVector( vectors );
			}
			
			ASSERT_EQ( AllocStatistics::totalAllocated(), 0 );
		}
	}
}