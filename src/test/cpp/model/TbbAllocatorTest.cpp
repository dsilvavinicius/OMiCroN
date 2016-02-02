#include <gtest/gtest.h>
#include <iostream>
#include <omp.h>
#include <chrono>
#include "Point.h"
#include "TbbAllocator.h"
#include "Profiler.h"

using namespace std;
using namespace util;

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
			
			auto start = Profiler::now();
			
			int nThreads = 8;
			
			{
				TestVector vectors; initVector( vectors, nThreads );
			
				int vectorsPerThread = 10000;
				int pointsPerIter = 3500;
				#pragma omp parallel for
				for( int i = 0; i < nThreads; i++ )
				{
					int threadIdx = omp_get_thread_num();
					ThreadVector threadVector; initVector( threadVector, vectorsPerThread );
					
					int j = 0;
					while( j < vectorsPerThread )
					{
						PointVector points; initVector( points, pointsPerIter );
						threadVector[ j++ ] = points;
					}
					
					vectors[ threadIdx ] = threadVector;
				}
				
				size_t expected = 	nThreads * (
										sizeof( ThreadVector )
										+ vectorsPerThread * (
											sizeof( PointVector ) + pointsPerIter * sizeof( Point )
										)
									);
				size_t allocated = AllocStatistics::totalAllocated();
				cout << "Expected (b): " << expected << ", Allocated (b): " << allocated << ", fragmentation (%): "
					 << float( allocated - expected ) / expected << endl << endl;
					 
				#pragma omp parallel for
				for( int i = 0; i < nThreads; i++ )
				{
					int threadIdx = omp_get_thread_num();
					int j = 0;
					while( j < vectorsPerThread )
					{
						releaseVector( vectors[ threadIdx ][ j++ ] );
					}
					
					releaseVector( vectors[ threadIdx ] );
				}
				
				releaseVector( vectors );
			}
			
			cout << "Elapsed time (ms): " << Profiler::elapsedTime( start ) << endl << endl;
			
			ASSERT_EQ( AllocStatistics::totalAllocated(), 0 );
		}
	}
}