#include <gtest/gtest.h>
#include "BitMapMemoryManager.h"
#include "OctreeMapTypes.h"
#include <InnerNode.h>
#include <MemoryManagerTypes.h>
#include <BitMapAllocator.h>
#include <MemoryUtils.h>
#include <qabstractitemmodel.h>

namespace model
{
	namespace test
	{
        class BitMapMemoryManagerTest : public ::testing::Test
		{};
		
		TEST_F( BitMapMemoryManagerTest, ManagedTypes0 )
		{
			using Morton = ShallowMortonCode;
			using MortonPtr = shared_ptr< Morton >;
			using Point = model::Point;
			using PointPtr = shared_ptr< Point >;
			using PointVector = vector< PointPtr >;
			using Inner = InnerNode< PointVector >;
			using InnerPtr = shared_ptr< Inner >;
			using Leaf = LeafNode< PointVector >;
			using LeafPtr = shared_ptr< Leaf >;
			using OctreeMap = model::OctreeMap< Morton >;
			
			uint nNodes = 500000u;
			uint nPoints = 2u * nNodes;
			size_t totalMortonsSize = nNodes * sizeof( Morton );
			size_t totalPointsSize = nPoints * sizeof( Point );
			size_t totalInnersSize = 0 * sizeof( Inner );
			size_t totalLeavesSize = nNodes * sizeof( Leaf );
			size_t maxMemToUse = totalMortonsSize + totalPointsSize + totalInnersSize + totalLeavesSize;
			
			BitMapMemoryManager< Morton, Point, Inner, Leaf >::initInstance( maxMemToUse );
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
		
		TEST_F( BitMapMemoryManagerTest, ManagedTypes1 )
		{
			using Morton = MediumMortonCode;
			using MortonPtr = shared_ptr< Morton >;
			using Point = model::ExtendedPoint;
			using PointPtr = shared_ptr< Point >;
			using PointVector = vector< PointPtr >;
			using Inner = InnerNode< PointVector >;
			using InnerPtr = shared_ptr< Inner >;
			using Leaf = LeafNode< PointVector >;
			using LeafPtr = shared_ptr< Leaf >;
			using OctreeMap = model::OctreeMap< Morton >;
			
			uint nNodes = 500000u;
			uint nPoints = 2u * nNodes;
			size_t totalMortonsSize = nNodes * sizeof( Morton );
			size_t totalPointsSize = nPoints * sizeof( Point );
			size_t totalInnersSize = nNodes * sizeof( Inner );
			size_t totalLeavesSize = 0;
			size_t maxMemToUse = totalMortonsSize + totalPointsSize + totalInnersSize + totalLeavesSize;
			
			BitMapMemoryManager< Morton, Point, Inner, Leaf >::initInstance( maxMemToUse );
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
		
		TEST_F( BitMapMemoryManagerTest, ManagedTypes2 )
		{
			using Morton = MediumMortonCode;
			using MortonPtr = shared_ptr< Morton >;
			using Point = model::ExtendedPoint;
			using PointPtr = shared_ptr< Point >;
			using PointVector = vector< PointPtr >;
			using Inner = InnerNode< ExtendedPointPtr >;
			using InnerPtr = shared_ptr< Inner >;
			using Leaf = LeafNode< PointVector >;
			using LeafPtr = shared_ptr< Leaf >;
			using OctreeMap = model::OctreeMap< Morton >;
			
			uint nNodes = 500000u;
			uint nPoints = nNodes;
			size_t totalMortonsSize = nNodes * sizeof( Morton );
			size_t totalPointsSize = nPoints * sizeof( Point );
			size_t totalInnersSize = nNodes * sizeof( Inner );
			size_t totalLeavesSize = 0;
			size_t maxMemToUse = totalMortonsSize + totalPointsSize + totalInnersSize + totalLeavesSize;
			
			BitMapMemoryManager< Morton, Point, Inner, Leaf >::initInstance( maxMemToUse );
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
		
		TEST_F( BitMapMemoryManagerTest, SimpleArrays )
		{
			using Morton = ShallowMortonCode;
			using Point = model::Point;
			using PointPtr = shared_ptr< Point >;
			using PointVector = vector< PointPtr >;
			using Inner = InnerNode< PointVector >;
			using Leaf = LeafNode< PointVector >;
			
			uint arraySizes[ 5 ] = { 100000u, 150000u, 50000u, 70000u, 120000u };
			uint arrayTotal = arraySizes[ 0 ] + arraySizes[ 1 ] + arraySizes[ 2 ] + arraySizes[ 3 ] + arraySizes[ 4 ];
			
			BitMapMemoryManager< Morton, Point, Inner, Leaf >::initInstance(
				arrayTotal * sizeof( Morton ) + arrayTotal * sizeof( Point ) + arrayTotal * sizeof( Inner )
				+ arrayTotal * sizeof( Leaf ) );
			
			Morton* mortonArrays[ 5 ];
			Point* pointArrays[ 5 ];
			Inner* innerArrays[ 5 ];
			Leaf* leafArrays[ 5 ];
			
			for( int i = 0; i < 5; ++i )
			{
				mortonArrays[ i ] = new Morton[ arraySizes[ i ] ];
				pointArrays[ i ] = new Point[ arraySizes[ i ] ];
				innerArrays[ i ] = new Inner[ arraySizes[ i ] ];
				leafArrays[ i ] = new Leaf[ arraySizes[ i ] ];
			}
			
			// Checking if arrays are being allocated to pointers as expected.
			uint sameBlockElements = arraySizes[ 0 ];
			for( int i = 1; i < 5; ++i )
			{
				sameBlockElements += arraySizes[ i ];
				if( sameBlockElements > BIT_MAP_SIZE )
				{
					sameBlockElements = arraySizes[ i ];
				}
				else
				{
					ASSERT_EQ( mortonArrays[ i ], mortonArrays[ i - 1 ] + arraySizes[ i - 1 ] );
					ASSERT_EQ( pointArrays[ i ], pointArrays[ i - 1 ] + arraySizes[ i - 1 ] );
					ASSERT_EQ( innerArrays[ i ], innerArrays[ i - 1 ] + arraySizes[ i - 1 ] );
					ASSERT_EQ( leafArrays[ i ], leafArrays[ i - 1 ] + arraySizes[ i - 1 ] );
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
				delete[] mortonArrays[ i ];
				delete[] pointArrays[ i ];
				delete[] innerArrays[ i ];
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
			
			// Creating arrays after deletions in order to check if memory is being reused correctly.
			for( int i = 0; i < 3; ++i )
			{
				mortonArrays[ i ] = new Morton[ arraySizes[ i ] ];
				pointArrays[ i ] = new Point[ arraySizes[ i ] ];
				innerArrays[ i ] = new Inner[ arraySizes[ i ] ];
				leafArrays[ i ] = new Leaf[ arraySizes[ i ] ];
				
				ASSERT_EQ( mortonArrays[ i ], expectedMortonArrays[ i ] );
				ASSERT_EQ( pointArrays[ i ], expectedPointArrays[ i ] );
				ASSERT_EQ( innerArrays[ i ], expectedInnerArrays[ i ] );
				ASSERT_EQ( leafArrays[ i ], expectedLeafArrays[ i ] );
			}
			
			// Deleting all arrays to setup the case where the pool could be lost because all ArrayMemoryInfo are erased.
			for( int i = 0; i < 5; ++i )
			{
				delete[] mortonArrays[ i ];
				delete[] pointArrays[ i ];
				delete[] innerArrays[ i ];
				delete[] leafArrays[ i ];
			}
			
			// Alloc again to check if the case aforementioned is not occurring and the pool is indeed being reused.
			for( int i = 0; i < 5; ++i )
			{
				mortonArrays[ i ] = new Morton[ arraySizes[ i ] ];
				pointArrays[ i ] = new Point[ arraySizes[ i ] ];
				innerArrays[ i ] = new Inner[ arraySizes[ i ] ];
				leafArrays[ i ] = new Leaf[ arraySizes[ i ] ];
				
				ASSERT_EQ( mortonArrays[ i ], expectedMortonArrays[ i ] );
				ASSERT_EQ( pointArrays[ i ], expectedPointArrays[ i ] );
				ASSERT_EQ( innerArrays[ i ], expectedInnerArrays[ i ] );
				ASSERT_EQ( leafArrays[ i ], expectedLeafArrays[ i ] );
			}
			
			for( int i = 0; i < 5; ++i )
			{
				delete[] mortonArrays[ i ];
				delete[] pointArrays[ i ];
				delete[] innerArrays[ i ];
				delete[] leafArrays[ i ];
			}
		}
		
		TEST_F( BitMapMemoryManagerTest, ComplexArrays )
		{
			using Morton = MediumMortonCode;
			using Point = model::ExtendedPoint;
			using PointPtr = shared_ptr< Point >;
			using PointVector = vector< PointPtr >;
			using Inner = InnerNode< PointVector >;
			using Leaf = LeafNode< PointVector >;
			
			uint arraySizes[ 5 ] = { 100000u, 150000u, 50000u, 70000u, 120000u };
			uint arrayTotal = arraySizes[ 0 ] + arraySizes[ 1 ] + arraySizes[ 2 ] + arraySizes[ 3 ] + arraySizes[ 4 ];
			
			BitMapMemoryManager< Morton, Point, Inner, Leaf >::initInstance(
				arrayTotal * sizeof( Morton ) + arrayTotal * sizeof( Point ) + arrayTotal * sizeof( Inner )
				+ arrayTotal * sizeof( Leaf ) );
			
			Morton* mortonArrays[ 5 ];
			Point* pointArrays[ 5 ];
			Inner* innerArrays[ 5 ];
			Leaf* leafArrays[ 5 ];
			
			for( int i = 0; i < 5; ++i )
			{
				mortonArrays[ i ] = new Morton[ arraySizes[ i ] ];
				pointArrays[ i ] = new Point[ arraySizes[ i ] ];
				innerArrays[ i ] = new Inner[ arraySizes[ i ] ];
				leafArrays[ i ] = new Leaf[ arraySizes[ i ] ];
			}
			
			// Checking if arrays are being allocated to pointers as expected.
			uint sameBlockElements = arraySizes[ 0 ];
			for( int i = 1; i < 5; ++i )
			{
				sameBlockElements += arraySizes[ i ];
				if( sameBlockElements > BIT_MAP_SIZE )
				{
					sameBlockElements = arraySizes[ i ];
				}
				else
				{
					ASSERT_EQ( mortonArrays[ i ], mortonArrays[ i - 1 ] + arraySizes[ i - 1 ] );
					ASSERT_EQ( pointArrays[ i ], pointArrays[ i - 1 ] + arraySizes[ i - 1 ] );
					ASSERT_EQ( innerArrays[ i ], innerArrays[ i - 1 ] + arraySizes[ i - 1 ] );
					ASSERT_EQ( leafArrays[ i ], leafArrays[ i - 1 ] + arraySizes[ i - 1 ] );
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
				delete[] mortonArrays[ i ];
				delete[] pointArrays[ i ];
				delete[] innerArrays[ i ];
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
			
			// Creating arrays after deletions in order to check if memory is being reused correctly.
			for( int i = 0; i < 3; ++i )
			{
				mortonArrays[ i ] = new Morton[ arraySizes[ i ] ];
				pointArrays[ i ] = new Point[ arraySizes[ i ] ];
				innerArrays[ i ] = new Inner[ arraySizes[ i ] ];
				leafArrays[ i ] = new Leaf[ arraySizes[ i ] ];
				
				ASSERT_EQ( mortonArrays[ i ], expectedMortonArrays[ i ] );
				ASSERT_EQ( pointArrays[ i ], expectedPointArrays[ i ] );
				ASSERT_EQ( innerArrays[ i ], expectedInnerArrays[ i ] );
				ASSERT_EQ( leafArrays[ i ], expectedLeafArrays[ i ] );
			}
			
			// Deleting all arrays to setup the case where the pool could be lost because all ArrayMemoryInfo are erased.
			for( int i = 0; i < 5; ++i )
			{
				delete[] mortonArrays[ i ];
				delete[] pointArrays[ i ];
				delete[] innerArrays[ i ];
				delete[] leafArrays[ i ];
			}
			
			// Alloc again to check if the case aforementioned is not occurring and the pool is indeed being reused.
			for( int i = 0; i < 5; ++i )
			{
				mortonArrays[ i ] = new Morton[ arraySizes[ i ] ];
				pointArrays[ i ] = new Point[ arraySizes[ i ] ];
				innerArrays[ i ] = new Inner[ arraySizes[ i ] ];
				leafArrays[ i ] = new Leaf[ arraySizes[ i ] ];
				
				ASSERT_EQ( mortonArrays[ i ], expectedMortonArrays[ i ] );
				ASSERT_EQ( pointArrays[ i ], expectedPointArrays[ i ] );
				ASSERT_EQ( innerArrays[ i ], expectedInnerArrays[ i ] );
				ASSERT_EQ( leafArrays[ i ], expectedLeafArrays[ i ] );
			}
			
			for( int i = 0; i < 5; ++i )
			{
				delete[] mortonArrays[ i ];
				delete[] pointArrays[ i ];
				delete[] innerArrays[ i ];
				delete[] leafArrays[ i ];
			}
		}
		
		TEST_F( BitMapMemoryManagerTest, AllocatorShallowPointVector )
		{
			using Morton = ShallowMortonCode;
			using MortonPtr = shared_ptr< Morton >;
			using MortonPtrInternals = PtrInternals< Morton, BitMapAllocator< Morton > >;
			using Point = model::Point;
			using PointPtr = shared_ptr< Point >;
			using PointPtrInternals = PtrInternals< Point, BitMapAllocator< Point > >;
			using PointVector = vector< PointPtr, BitMapAllocator< PointPtr > >;
			using Inner = InnerNode< PointVector >;
			using InnerPtr = shared_ptr< Inner >;
			using InnerPtrInternals = PtrInternals< Inner, BitMapAllocator< Inner > >;
			using Leaf = LeafNode< PointVector >;
			using LeafPtr = shared_ptr< Leaf >;
			using LeafPtrInternals = PtrInternals< Leaf, BitMapAllocator< Leaf > >;
			using OctreeMap = map< 	MortonPtr, OctreeNodePtr, ShallowMortonComparator,
									BitMapAllocator< pair< const MortonPtr, OctreeNodePtr > > >;
			using MapInternals = model::MapInternals< Morton >;
			
			int nPoints = 10000;
			size_t expectedMemUsage = 	nPoints * ( 3 * sizeof( PointPtr ) + sizeof( PointPtrInternals ) )
										+ 2 * sizeof( MortonPtrInternals ) + sizeof( InnerPtrInternals )
										+ sizeof( LeafPtrInternals ) + 2 * sizeof( MapInternals );
			BitMapMemoryManager< Morton, Point, Inner, Leaf >::initInstance( expectedMemUsage );
			
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
		
		TEST_F( BitMapMemoryManagerTest, AllocatorShallowPointPtr )
		{
			using Morton = ShallowMortonCode;
			using MortonPtr = shared_ptr< Morton >;
			using MortonPtrInternals = PtrInternals< Morton, BitMapAllocator< Morton > >;
			using Point = model::Point;
			using PointPtr = shared_ptr< Point >;
			using PointPtrInternals = PtrInternals< Point, BitMapAllocator< Point > >;
			using PointVector = vector< PointPtr, BitMapAllocator< PointPtr > >;
			using Inner = InnerNode< PointPtr >;
			using InnerPtr = shared_ptr< Inner >;
			using InnerPtrInternals = PtrInternals< Inner, BitMapAllocator< Inner > >;
			using Leaf = LeafNode< PointVector >;
			using LeafPtr = shared_ptr< Leaf >;
			using LeafPtrInternals = PtrInternals< Leaf, BitMapAllocator< Leaf > >;
			
			using OctreeMap = map< 	MortonPtr, OctreeNodePtr, ShallowMortonComparator,
									BitMapAllocator< pair< const MortonPtr, OctreeNodePtr > > >;
			using MapInternals = model::MapInternals< Morton >;
			
			int nPoints = 10000;
			size_t expectedMemUsage = 	nPoints * ( 2 * sizeof( PointPtr ) + sizeof( PointPtrInternals ) )
										+ sizeof( PointPtrInternals ) + 2 * sizeof( MortonPtrInternals )
										+ sizeof( InnerPtrInternals ) + sizeof( LeafPtrInternals )
										+ 2 * sizeof( MapInternals );
			BitMapMemoryManager< Morton, Point, Inner, Leaf >::initInstance( expectedMemUsage );
			
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
		
		TEST_F( BitMapMemoryManagerTest, AllocatorsMediumExtendedIndex )
		{
			using Morton = MediumMortonCode;
			using MortonPtr = shared_ptr< Morton >;
			using MortonPtrInternals = PtrInternals< Morton, BitMapAllocator< Morton > >;
			using Point = ExtendedPoint;
			using PointPtr = shared_ptr< Point >;
			using PointPtrInternals = PtrInternals< Point, BitMapAllocator< Point > >;
			using PointVector = vector< PointPtr, BitMapAllocator< PointPtr > >;
			using IndexVector = vector< Index, BitMapAllocator< Index > >;
			using Inner = InnerNode< IndexVector >;
			using InnerPtr = shared_ptr< Inner >;
			using InnerPtrInternals = PtrInternals< Inner, BitMapAllocator< Inner > >;
			using Leaf = LeafNode< IndexVector >;
			using LeafPtr = shared_ptr< Leaf >;
			using LeafPtrInternals = PtrInternals< Leaf, BitMapAllocator< Leaf > >;
			
			using OctreeMap = map< 	MortonPtr, OctreeNodePtr, MediumMortonComparator,
									BitMapAllocator< pair< const MortonPtr, OctreeNodePtr > > >;
			using MapInternals = model::MapInternals< Morton >;
			
			int nPoints = 10000;
			size_t expectedMemUsage = 	nPoints * ( 3 * sizeof( Index ) + sizeof( PointPtrInternals ) + sizeof( PointPtr ) )
										+ 2 * sizeof( MortonPtrInternals ) + sizeof( InnerPtrInternals )
										+ sizeof( LeafPtrInternals ) + 2 * sizeof( MapInternals );
			BitMapMemoryManager< Morton, Point, Inner, Leaf >::initInstance( expectedMemUsage );
			
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