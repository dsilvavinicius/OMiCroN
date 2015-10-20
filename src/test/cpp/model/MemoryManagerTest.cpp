#include <gtest/gtest.h>
#include "BitMapMemoryManager.h"
#include "OctreeMapTypes.h"
#include <InnerNode.h>
#include <MemoryManagerTypes.h>
#include <ManagedAllocator.h>
#include <MemoryUtils.h>
#include <qabstractitemmodel.h>

namespace model
{
	namespace test
	{
		// Hacks to allow overloading.
		typedef struct BitMapManagerType{} BitMapManagerType;
		typedef struct TLSFManagerType{} TLSFManagerType;
		
		template< typename Morton, typename Point, typename Inner, typename Leaf >
		void initManager( const size_t& maxMemory, const BitMapManagerType& )
		{
			BitMapMemoryManager< Morton, Point, Inner, Leaf >::initInstance( maxMemory );
		}
		
		template< typename Morton, typename Point, typename Inner, typename Leaf >
		void initManager( const size_t& maxMemory, const TLSFManagerType& )
		{
			TLSFManager< Morton, Point, Inner, Leaf >::initInstance( maxMemory );
		}
		
		template< typename M >
        class MemoryManagerTest
        :  public ::testing::Test
        {};
		
		using testing::Types;
		
		typedef Types< /*BitMapManagerType,*/ TLSFManagerType > Implementations;
		
		TYPED_TEST_CASE( MemoryManagerTest, Implementations );
		
        //class BitMapMemoryManagerTest : public ::testing::Test
		//{};
		
		TYPED_TEST( MemoryManagerTest, ManagedTypes0 )
		{
			using Morton = ShallowMortonCode;
			using MortonPtr = shared_ptr< Morton >;
			using MortonPtrInternals = PtrInternals< Morton, ManagedAllocator< Morton > >;
			
			using Point = model::Point;
			using PointPtr = shared_ptr< Point >;
			using PointPtrInternals = PtrInternals< Point, ManagedAllocator< Point > >;
			using PointVector = vector< PointPtr, ManagedAllocator< PointPtr > >;
			
			using Inner = InnerNode< PointVector >;
			using InnerPtr = shared_ptr< Inner >;
			using InnerPtrInternals = PtrInternals< Inner, ManagedAllocator< Inner > >;
			
			using Leaf = LeafNode< PointVector >;
			using LeafPtr = shared_ptr< Leaf >;
			using LeafPtrInternals = PtrInternals< Leaf, ManagedAllocator< Leaf > >;
			
			using OctreeMap = model::OctreeMap< Morton >;
			using MapInternals = model::MapInternals< Morton >;
			
			uint nNodes = 500000u;
			uint nPoints = 2u * nNodes;
			size_t maxMemToUse = nNodes * ( sizeof( MortonPtrInternals ) + sizeof( LeafPtrInternals )
								+ sizeof( MapInternals ) + 3 * ( sizeof( PointPtr ) ) )
								+ nPoints * sizeof( PointPtrInternals );
			
			initManager< Morton, Point, Inner, Leaf >( maxMemToUse, TypeParam() );
			//BitMapMemoryManager< Morton, Point, Inner, Leaf >::initInstance( maxMemToUse );
			IMemoryManager& manager = SingletonMemoryManager::instance();
			
			ASSERT_EQ( 0, manager.usedMemory() );
			
			OctreeMap map;
			
			for( unsigned int i = 0u; i < nNodes; ++i )
			{
				MortonPtr mortonCode = makeManaged< Morton >();
				mortonCode->build( i );
				
				LeafPtr node = makeManaged< Leaf >();
				PointPtr p0 = makeManaged< Point >();
				PointPtr p1 = makeManaged< Point >();
				node->setContents( PointVector( { p0, p1 } ) );
				
				map[ mortonCode ] = node;
			}
			
			ASSERT_EQ( maxMemToUse, manager.usedMemory() );
			
			for( unsigned int i = 0u; i < nNodes; ++i )
			{
				MortonPtr expected = makeManaged< Morton >();
				expected->build( i );
				
				ASSERT_TRUE( map.find( expected ) != map.end() );
			}
			
			map.clear();
			
			ASSERT_EQ( 0, manager.usedMemory() );
		}
		
		TYPED_TEST( MemoryManagerTest, ManagedTypes1 )
		{
			using Morton = MediumMortonCode;
			using MortonPtr = shared_ptr< Morton >;
			using MortonPtrInternals = PtrInternals< Morton, ManagedAllocator< Morton > >;
			
			using Point = model::ExtendedPoint;
			using PointPtr = shared_ptr< Point >;
			using PointPtrInternals = PtrInternals< Point, ManagedAllocator< Point > >;
			using PointVector = vector< PointPtr, ManagedAllocator< PointPtr > >;
			
			using Inner = InnerNode< PointVector >;
			using InnerPtr = shared_ptr< Inner >;
			using InnerPtrInternals = PtrInternals< Inner, ManagedAllocator< Inner > >;
			using Leaf = LeafNode< PointVector >;
			
			using OctreeMap = model::OctreeMap< Morton >;
			using MapInternals = model::MapInternals< Morton >;
			
			uint nNodes = 500000u;
			uint nPoints = 2u * nNodes;
			size_t maxMemToUse = nNodes * ( sizeof( MortonPtrInternals ) + sizeof( InnerPtrInternals )
								+ sizeof( MapInternals ) + 3 * ( sizeof( PointPtr ) ) )
								+ nPoints * sizeof( PointPtrInternals );
			
			initManager< Morton, Point, Inner, Leaf >( maxMemToUse, TypeParam() );
			//BitMapMemoryManager< Morton, Point, Inner, Leaf >::initInstance( maxMemToUse );
			IMemoryManager& manager = SingletonMemoryManager::instance();
			
			ASSERT_EQ( 0, manager.usedMemory() );
			
			OctreeMap map;
			
			for( unsigned int i = 0u; i < nNodes; ++i )
			{
				MortonPtr mortonCode = makeManaged< Morton >();
				mortonCode->build( i );
				
				InnerPtr node = makeManaged< Inner >();
				PointPtr p0 = makeManaged< Point >();
				PointPtr p1 = makeManaged< Point >();
				node->setContents( PointVector( { p0, p1 } ) );
				
				map[ mortonCode ] = node;
			}
			
			ASSERT_EQ( maxMemToUse, manager.usedMemory() );
			
			for( unsigned int i = 0u; i < nNodes; ++i )
			{
				MortonPtr expected = makeManaged< Morton >();
				expected->build( i );
				
				ASSERT_TRUE( map.find( expected ) != map.end() );
			}
			
			map.clear();
			
			ASSERT_EQ( 0, manager.usedMemory() );
		}
		
		TYPED_TEST( MemoryManagerTest, ManagedTypes2 )
		{
			using Morton = MediumMortonCode;
			using MortonPtr = shared_ptr< Morton >;
			using MortonPtrInternals = PtrInternals< Morton, ManagedAllocator< Morton > >;
			
			using Point = model::ExtendedPoint;
			using PointPtr = shared_ptr< Point >;
			using PointPtrInternals = PtrInternals< Point, ManagedAllocator< Point > >;
			using PointVector = vector< PointPtr, ManagedAllocator< PointPtr > >;
			
			using Inner = InnerNode< ExtendedPointPtr >;
			using InnerPtr = shared_ptr< Inner >;
			using InnerPtrInternals = PtrInternals< Inner, ManagedAllocator< Inner > >;
			
			using Leaf = LeafNode< PointVector >;
			using LeafPtr = shared_ptr< Leaf >;
			
			using OctreeMap = model::OctreeMap< Morton >;
			using MapInternals = model::MapInternals< Morton >;
			
			uint nNodes = 500000u;
			size_t maxMemToUse = nNodes * ( sizeof( MortonPtrInternals ) + sizeof( InnerPtrInternals )
								+ sizeof( MapInternals ) + sizeof( PointPtrInternals ) );
			
			//BitMapMemoryManager< Morton, Point, Inner, Leaf >::initInstance( maxMemToUse );
			initManager< Morton, Point, Inner, Leaf >( maxMemToUse, TypeParam() );
			IMemoryManager& manager = SingletonMemoryManager::instance();
			
			ASSERT_EQ( 0, manager.usedMemory() );
			
			OctreeMap map;
			
			for( unsigned int i = 0u; i < nNodes; ++i )
			{
				MortonPtr mortonCode = makeManaged< Morton >();
				mortonCode->build( i );
				
				InnerPtr node = makeManaged< Inner >();
				PointPtr point = makeManaged< Point >();
				node->setContents( point );
				
				map[ mortonCode ] = node;
			}
			
			ASSERT_EQ( maxMemToUse, manager.usedMemory() );
			
			for( unsigned int i = 0u; i < nNodes; ++i )
			{
				MortonPtr expected = makeManaged< Morton >();
				expected->build( i );
				
				ASSERT_TRUE( map.find( expected ) != map.end() );
			}
			
			map.clear();
			
			ASSERT_EQ( 0, manager.usedMemory() );
		}
		
		TYPED_TEST( MemoryManagerTest, SimpleArrays )
		{
			using Morton = ShallowMortonCode;
			using Point = model::Point;
			using PointPtr = shared_ptr< Point >;
			using PointVector = vector< PointPtr, ManagedAllocator< PointPtr > >;
			using Inner = InnerNode< PointVector >;
			using Leaf = LeafNode< PointVector >;
			
			uint arraySizes[ 5 ] = { 100000u, 150000u, 50000u, 70000u, 120000u };
			
			//BitMapMemoryManager< Morton, Point, Inner, Leaf >::initInstance( 1024 * 1024 );
			initManager< Morton, Point, Inner, Leaf >( 1024 * 1024, TypeParam() );
			
			Morton* mortonArrays[ 5 ];
			Point* pointArrays[ 5 ];
			Inner* innerArrays[ 5 ];
			Leaf* leafArrays[ 5 ];
			
			for( int i = 0; i < 5; ++i )
			{
				//cout << "Morton" << endl << endl;
				
				mortonArrays[ i ] = new Morton[ arraySizes[ i ] ];
				
				//cout << "Point" << endl << endl;
				
				pointArrays[ i ] = new Point[ arraySizes[ i ] ];
				
				//cout << "Inner" << endl << endl;
				
				innerArrays[ i ] = new Inner[ arraySizes[ i ] ];
				
				//cout << "Leaf" << endl << endl;
				
				leafArrays[ i ] = new Leaf[ arraySizes[ i ] ];
			}
			
			// Checking if arrays are being allocated to pointers as expected.
			uint sameBlockElements = arraySizes[ 0 ] + 1;
			for( int i = 1; i < 5; ++i )
			{
				sameBlockElements += arraySizes[ i ] + 1;
				if( sameBlockElements > BIT_MAP_SIZE )
				{
					sameBlockElements = arraySizes[ i ] + 1;
				}
				else
				{
					//cout << "array " << i << endl << endl;
					ASSERT_EQ( mortonArrays[ i ] - mortonArrays[ i - 1 ], arraySizes[ i - 1 ] + 1 );
					ASSERT_EQ( pointArrays[ i ] - pointArrays[ i - 1 ], arraySizes[ i - 1 ] + 1 );
					ASSERT_EQ( innerArrays[ i ] - innerArrays[ i - 1 ], arraySizes[ i - 1 ] + 1 );
					ASSERT_EQ( leafArrays[ i ] - leafArrays[ i - 1 ], arraySizes[ i - 1 ] + 1 );
				}
			}
			
			// After validation, the acquired pointers are set as the expected value for allocation return.
			Morton* expectedMortonArrays[ 5 ];
			Point* expectedPointArrays[ 5 ];
			Inner* expectedInnerArrays[ 5 ];
			Leaf* expectedLeafArrays[ 5 ];
			
			for( int i = 0; i < 5; ++i )
			{
				expectedMortonArrays[ i ] = mortonArrays[ i ];
				expectedPointArrays[ i ] = pointArrays[ i ];
				expectedInnerArrays[ i ] = innerArrays[ i ];
				expectedLeafArrays[ i ] = leafArrays[ i ];
			}
			
			// Deleting some arrays in order to check if remaining pointers are correct afterwards.
			for( int i = 0; i < 3; ++i )
			{
				//cout << "Delete morton" << endl << endl;
				
				delete[] mortonArrays[ i ];
				
				//cout << "Delete point" << endl << endl;
				
				delete[] pointArrays[ i ];
				
				//cout << "Delete inner" << endl << endl;
				
				delete[] innerArrays[ i ];
				
				//cout << "Delete leaf" << endl << endl;
				
				delete[] leafArrays[ i ];
				
				// Dereferencing attempt to check integrity after deletes.
				for( int j = i + 1; j < 5; ++j )
				{
					ASSERT_NO_THROW( Morton morton = mortonArrays[ j ][ 0 ] );
					ASSERT_NO_THROW( Point point = pointArrays[ j ][ 0 ] );
					ASSERT_NO_THROW( Inner point = innerArrays[ j ][ 0 ] );
					ASSERT_NO_THROW( Leaf point = leafArrays[ j ][ 0 ] );
				}
			}
			
			// Creating arrays after deletions in order to check if memory is handled correctly after deletions.
			for( int i = 0; i < 3; ++i )
			{
				mortonArrays[ i ] = new Morton[ arraySizes[ i ] ];
				pointArrays[ i ] = new Point[ arraySizes[ i ] ];
				innerArrays[ i ] = new Inner[ arraySizes[ i ] ];
				leafArrays[ i ] = new Leaf[ arraySizes[ i ] ];
				
				int modPrev = ( ( i - 1 ) % 5 + 5 ) % 5;
				
				ASSERT_EQ( mortonArrays[ i ] - mortonArrays[ modPrev  ] - arraySizes[ modPrev ] - 1, 0 );
				ASSERT_EQ( pointArrays[ i ] - pointArrays[ modPrev ] - arraySizes[ modPrev ] - 1, 0 );
				ASSERT_EQ( innerArrays[ i ] - innerArrays[ modPrev ] - arraySizes[ modPrev ] - 1, 0 );
				ASSERT_EQ( leafArrays[ i ] - leafArrays[ modPrev ] - arraySizes[ modPrev ] - 1, 0 );
			}
			
			// Deleting all arrays to setup the case where the pool could be lost because all ArrayMemoryInfo are erased.
			for( int i = 0; i < 5; ++i )
			{
				//cout << "Delete morton" << endl << endl;
				delete[] mortonArrays[ i ];
				
				//cout << "Delete point" << endl << endl;
				delete[] pointArrays[ i ];
				
				//cout << "Delete inner" << endl << endl;
				delete[] innerArrays[ i ];
				
				//cout << "Delete leaf" << endl << endl;
				delete[] leafArrays[ i ];
			}
			
			// Alloc again to check if memory is reused after deletion.
			for( int i = 0; i < 5; ++i )
			{
				mortonArrays[ i ] = new Morton[ arraySizes[ i ] ];
				pointArrays[ i ] = new Point[ arraySizes[ i ] ];
				innerArrays[ i ] = new Inner[ arraySizes[ i ] ];
				leafArrays[ i ] = new Leaf[ arraySizes[ i ] ];
				
				ASSERT_EQ( mortonArrays[ i ] - expectedMortonArrays[ i ], 0 );
				ASSERT_EQ( pointArrays[ i ] - expectedPointArrays[ i ], 0 );
				ASSERT_EQ( innerArrays[ i ] - expectedInnerArrays[ i ], 0 );
				ASSERT_EQ( leafArrays[ i ] - expectedLeafArrays[ i ], 0 );
			}
			
			for( int i = 0; i < 5; ++i )
			{
				delete[] mortonArrays[ i ];
				delete[] pointArrays[ i ];
				delete[] innerArrays[ i ];
				delete[] leafArrays[ i ];
			}
		}
		
		TYPED_TEST( MemoryManagerTest, ComplexArrays )
		{
			using Morton = MediumMortonCode;
			using Point = model::ExtendedPoint;
			using PointPtr = shared_ptr< Point >;
			using PointVector = vector< PointPtr, ManagedAllocator< PointPtr > >;
			using Inner = InnerNode< PointVector >;
			using Leaf = LeafNode< PointVector >;
			
			uint arraySizes[ 5 ] = { 100000u, 150000u, 50000u, 70000u, 120000u };
			
			//BitMapMemoryManager< Morton, Point, Inner, Leaf >::initInstance( 1024 * 1024 );
			initManager< Morton, Point, Inner, Leaf >( 1024 * 1024, TypeParam() );
			
			Morton* mortonArrays[ 5 ];
			Point* pointArrays[ 5 ];
			Inner* innerArrays[ 5 ];
			Leaf* leafArrays[ 5 ];
			
			for( int i = 0; i < 5; ++i )
			{
				//cout << "Morton" << endl << endl;
				
				mortonArrays[ i ] = new Morton[ arraySizes[ i ] ];
				
				//cout << "Point" << endl << endl;
				
				pointArrays[ i ] = new Point[ arraySizes[ i ] ];
				
				//cout << "Inner" << endl << endl;
				
				innerArrays[ i ] = new Inner[ arraySizes[ i ] ];
				
				//cout << "Leaf" << endl << endl;
				
				leafArrays[ i ] = new Leaf[ arraySizes[ i ] ];
			}
			
			// Checking if arrays are being allocated to pointers as expected.
			uint sameBlockElements = arraySizes[ 0 ] + 1;
			for( int i = 1; i < 5; ++i )
			{
				sameBlockElements += arraySizes[ i ] + 1;
				if( sameBlockElements > BIT_MAP_SIZE )
				{
					sameBlockElements = arraySizes[ i ] + 1;
				}
				else
				{
					//cout << "array " << i << endl << endl;
					ASSERT_EQ( mortonArrays[ i ] - mortonArrays[ i - 1 ], arraySizes[ i - 1 ] + 1 );
					ASSERT_EQ( pointArrays[ i ] - pointArrays[ i - 1 ], arraySizes[ i - 1 ] + 1 );
					ASSERT_EQ( innerArrays[ i ] - innerArrays[ i - 1 ], arraySizes[ i - 1 ] + 1 );
					ASSERT_EQ( leafArrays[ i ] - leafArrays[ i - 1 ], arraySizes[ i - 1 ] + 1 );
				}
			}
			
			// After validation, the acquired pointers are set as the expected value for allocation return.
			Morton* expectedMortonArrays[ 5 ];
			Point* expectedPointArrays[ 5 ];
			Inner* expectedInnerArrays[ 5 ];
			Leaf* expectedLeafArrays[ 5 ];
			
			for( int i = 0; i < 5; ++i )
			{
				expectedMortonArrays[ i ] = mortonArrays[ i ];
				expectedPointArrays[ i ] = pointArrays[ i ];
				expectedInnerArrays[ i ] = innerArrays[ i ];
				expectedLeafArrays[ i ] = leafArrays[ i ];
			}
			
			// Deleting some arrays in order to check if remaining pointers are correct afterwards.
			for( int i = 0; i < 3; ++i )
			{
				//cout << "Delete morton" << endl << endl;
				
				delete[] mortonArrays[ i ];
				
				//cout << "Delete point" << endl << endl;
				
				delete[] pointArrays[ i ];
				
				//cout << "Delete inner" << endl << endl;
				
				delete[] innerArrays[ i ];
				
				//cout << "Delete leaf" << endl << endl;
				
				delete[] leafArrays[ i ];
				
				// Dereferencing attempt to check integrity after deletes.
				for( int j = i + 1; j < 5; ++j )
				{
					ASSERT_NO_THROW( Morton morton = mortonArrays[ j ][ 0 ] );
					ASSERT_NO_THROW( Point point = pointArrays[ j ][ 0 ] );
					ASSERT_NO_THROW( Inner point = innerArrays[ j ][ 0 ] );
					ASSERT_NO_THROW( Leaf point = leafArrays[ j ][ 0 ] );
				}
			}
			
			// Creating arrays after deletions in order to check if memory is handled correctly after deletions.
			for( int i = 0; i < 3; ++i )
			{
				mortonArrays[ i ] = new Morton[ arraySizes[ i ] ];
				pointArrays[ i ] = new Point[ arraySizes[ i ] ];
				innerArrays[ i ] = new Inner[ arraySizes[ i ] ];
				leafArrays[ i ] = new Leaf[ arraySizes[ i ] ];
				
				int modPrev = ( ( i - 1 ) % 5 + 5 ) % 5;
				
				ASSERT_EQ( mortonArrays[ i ] - mortonArrays[ modPrev  ] - arraySizes[ modPrev ] - 1, 0 );
				ASSERT_EQ( pointArrays[ i ] - pointArrays[ modPrev ] - arraySizes[ modPrev ] - 1, 0 );
				ASSERT_EQ( innerArrays[ i ] - innerArrays[ modPrev ] - arraySizes[ modPrev ] - 1, 0 );
				ASSERT_EQ( leafArrays[ i ] - leafArrays[ modPrev ] - arraySizes[ modPrev ] - 1, 0 );
			}
			
			// Deleting all arrays to setup the case where the pool could be lost because all ArrayMemoryInfo are erased.
			for( int i = 0; i < 5; ++i )
			{
				//cout << "Delete morton" << endl << endl;
				delete[] mortonArrays[ i ];
				
				//cout << "Delete point" << endl << endl;
				delete[] pointArrays[ i ];
				
				//cout << "Delete inner" << endl << endl;
				delete[] innerArrays[ i ];
				
				//cout << "Delete leaf" << endl << endl;
				delete[] leafArrays[ i ];
			}
			
			// Alloc again to check if memory is reused after deletion.
			for( int i = 0; i < 5; ++i )
			{
				mortonArrays[ i ] = new Morton[ arraySizes[ i ] ];
				pointArrays[ i ] = new Point[ arraySizes[ i ] ];
				innerArrays[ i ] = new Inner[ arraySizes[ i ] ];
				leafArrays[ i ] = new Leaf[ arraySizes[ i ] ];
				
				ASSERT_EQ( mortonArrays[ i ] - expectedMortonArrays[ i ], 0 );
				ASSERT_EQ( pointArrays[ i ] - expectedPointArrays[ i ], 0 );
				ASSERT_EQ( innerArrays[ i ] - expectedInnerArrays[ i ], 0 );
				ASSERT_EQ( leafArrays[ i ] - expectedLeafArrays[ i ], 0 );
			}
			
			for( int i = 0; i < 5; ++i )
			{
				delete[] mortonArrays[ i ];
				delete[] pointArrays[ i ];
				delete[] innerArrays[ i ];
				delete[] leafArrays[ i ];
			}
		}
		
		TYPED_TEST( MemoryManagerTest, AllocatorShallowPointVector )
		{
			using Morton = ShallowMortonCode;
			using MortonPtr = shared_ptr< Morton >;
			using MortonPtrInternals = PtrInternals< Morton, ManagedAllocator< Morton > >;
			
			using Point = model::Point;
			using PointPtr = shared_ptr< Point >;
			using PointPtrInternals = PtrInternals< Point, ManagedAllocator< Point > >;
			using PointVector = vector< PointPtr, ManagedAllocator< PointPtr > >;
			
			using Inner = InnerNode< PointVector >;
			using InnerPtr = shared_ptr< Inner >;
			using InnerPtrInternals = PtrInternals< Inner, ManagedAllocator< Inner > >;
			
			using Leaf = LeafNode< PointVector >;
			using LeafPtr = shared_ptr< Leaf >;
			using LeafPtrInternals = PtrInternals< Leaf, ManagedAllocator< Leaf > >;
			
			using OctreeMap = model::OctreeMap< Morton >;
			using MapInternals = model::MapInternals< Morton >;
			
			int nPoints = 10000;
			size_t expectedMemUsage = 	nPoints * ( 3 * sizeof( PointPtr ) + sizeof( PointPtrInternals ) )
										+ 3 * sizeof( PointPtr ) + 2 * sizeof( MortonPtrInternals )
										+ sizeof( InnerPtrInternals ) + sizeof( LeafPtrInternals )
										+ 2 * sizeof( MapInternals );
			
			//BitMapMemoryManager< Morton, Point, Inner, Leaf >::initInstance( expectedMemUsage );
			initManager< Morton, Point, Inner, Leaf >( expectedMemUsage, TypeParam() );
			
			PointVector points( nPoints );
			for( int i = 0; i < nPoints; ++i )
			{
				PointPtr point = makeManaged< Point >();
				points[ i ] = point;
			}
			
			//cout << "Manager after allocs: " << SingletonMemoryManager::instance() << endl;
			
			InnerPtr inner = makeManaged< Inner >();
			inner->setContents( points );
			
			LeafPtr leaf = makeManaged< Leaf >();
			leaf->setContents( points );
			
			MortonPtr morton0 = makeManaged< Morton >();
			MortonPtr morton1 = makeManaged< Morton >(); morton1->build( 0xF );
			OctreeMap map;
			map[ morton0 ] = inner;
			map[ morton1 ] = leaf;
			
			ASSERT_EQ( SingletonMemoryManager::instance().usedMemory(), expectedMemUsage );
		}
		
		TYPED_TEST( MemoryManagerTest, AllocatorShallowPointPtr )
		{
			using Morton = ShallowMortonCode;
			using MortonPtr = shared_ptr< Morton >;
			using MortonPtrInternals = PtrInternals< Morton, ManagedAllocator< Morton > >;
			
			using Point = model::Point;
			using PointPtr = shared_ptr< Point >;
			using PointPtrInternals = PtrInternals< Point, ManagedAllocator< Point > >;
			using PointVector = vector< PointPtr, ManagedAllocator< PointPtr > >;
			
			using Inner = InnerNode< PointPtr >;
			using InnerPtr = shared_ptr< Inner >;
			using InnerPtrInternals = PtrInternals< Inner, ManagedAllocator< Inner > >;
			
			using Leaf = LeafNode< PointVector >;
			using LeafPtr = shared_ptr< Leaf >;
			using LeafPtrInternals = PtrInternals< Leaf, ManagedAllocator< Leaf > >;
			
			using OctreeMap = model::OctreeMap< Morton >;
			using MapInternals = model::MapInternals< Morton >;
			
			int nPoints = 10000;
			size_t expectedMemUsage = 	nPoints * ( 2 * sizeof( PointPtr ) + sizeof( PointPtrInternals ) )
										+ 2 * sizeof( PointPtr ) + sizeof( PointPtrInternals )
										+ 2 * sizeof( MortonPtrInternals ) + sizeof( InnerPtrInternals )
										+ sizeof( LeafPtrInternals ) + 2 * sizeof( MapInternals );
			//BitMapMemoryManager< Morton, Point, Inner, Leaf >::initInstance( expectedMemUsage );
			initManager< Morton, Point, Inner, Leaf >( expectedMemUsage, TypeParam() );
			
			PointVector points( nPoints );
			for( int i = 0; i < nPoints; ++i )
			{
				PointPtr point = makeManaged< Point >();
				points[ i ] = point;
			}
			
			InnerPtr inner = makeManaged< Inner >();
			inner->setContents( makeManaged< Point >() );
			
			LeafPtr leaf = makeManaged< Leaf >();
			leaf->setContents( points );
			
			MortonPtr morton0 = makeManaged< Morton >();
			MortonPtr morton1 = makeManaged< Morton >(); morton1->build( 0xF );
			OctreeMap map;
			map[ morton0 ] = inner;
			map[ morton1 ] = leaf;
			
			ASSERT_EQ( SingletonMemoryManager::instance().usedMemory(), expectedMemUsage );
		}
		
		TYPED_TEST( MemoryManagerTest, AllocatorsMediumExtendedIndex )
		{
			using Morton = MediumMortonCode;
			using MortonPtr = shared_ptr< Morton >;
			using MortonPtrInternals = PtrInternals< Morton, ManagedAllocator< Morton > >;
			
			using Point = ExtendedPoint;
			using PointPtr = shared_ptr< Point >;
			using PointPtrInternals = PtrInternals< Point, ManagedAllocator< Point > >;
			
			using PointVector = vector< PointPtr, ManagedAllocator< PointPtr > >;
			using IndexVector = vector< Index, ManagedAllocator< Index > >;
			
			using Inner = InnerNode< IndexVector >;
			using InnerPtr = shared_ptr< Inner >;
			using InnerPtrInternals = PtrInternals< Inner, ManagedAllocator< Inner > >;
			
			using Leaf = LeafNode< IndexVector >;
			using LeafPtr = shared_ptr< Leaf >;
			using LeafPtrInternals = PtrInternals< Leaf, ManagedAllocator< Leaf > >;
			
			using OctreeMap = model::OctreeMap< Morton >;
			using MapInternals = model::MapInternals< Morton >;
			
			int nPoints = 10000;
			size_t expectedMemUsage = 	nPoints * ( sizeof( PointPtr ) + 3 * sizeof( Index ) + sizeof( PointPtrInternals ) )
										+ sizeof( PointPtr ) + 3 * sizeof( Index ) + 2 * sizeof( MortonPtrInternals )
										+ sizeof( InnerPtrInternals ) + sizeof( LeafPtrInternals )
										+ 2 * sizeof( MapInternals );
			//BitMapMemoryManager< Morton, Point, Inner, Leaf >::initInstance( expectedMemUsage );
			initManager< Morton, Point, Inner, Leaf >( expectedMemUsage, TypeParam() );
			
			PointVector points( nPoints );
			IndexVector indices( nPoints );
			for( int i = 0; i < nPoints; ++i )
			{
				points[ i ] = makeManaged< Point >();
				indices[ i ] = i;
			}
			
			InnerPtr inner = makeManaged< Inner >();
			inner->setContents( indices );
			
			LeafPtr leaf = makeManaged< Leaf >();
			leaf->setContents( indices );
			
			MortonPtr morton0 = makeManaged< Morton >();
			MortonPtr morton1 = makeManaged< Morton >(); morton1->build( 0xF );
			OctreeMap map;
			map[ morton0 ] = inner;
			map[ morton1 ] = leaf;
			
			ASSERT_EQ( SingletonMemoryManager::instance().usedMemory(), expectedMemUsage );
		}
	}
}