#include <gtest/gtest.h>
//#include <qabstractitemmodel.h>
#include "BitMapMemoryManager.h"
#include "OctreeMapTypes.h"
#include "OctreeNode.h"
#include "MemoryManagerTypes.h"
#include "ManagedAllocator.h"
#include "MemoryUtils.h"

namespace model
{
	namespace test
	{
		// Hacks to allow overloading.
		typedef struct BitMapManagerType{} BitMapManagerType;
		typedef struct TLSFManagerType{} TLSFManagerType;
		
		template< typename Morton, typename Point, typename Node >
		void initManager( const size_t& maxMemory, const BitMapManagerType& )
		{
			BitMapMemoryManager< Morton, Point, Node >::initInstance( maxMemory );
		}
		
		template< typename Morton, typename Point, typename Node >
		void initManager( const size_t& maxMemory, const TLSFManagerType& )
		{
			TLSFManager< Morton, Point, Node >::initInstance( maxMemory );
		}
		
		template< typename M >
        class MemoryManagerTest
        :  public ::testing::Test
        {};
		
		using testing::Types;
		
		typedef Types< /*BitMapManagerType,*/ TLSFManagerType > Implementations;
		
		TYPED_TEST_CASE( MemoryManagerTest, Implementations );
		
		TYPED_TEST( MemoryManagerTest, ManagedTypes0 )
		{
			using Morton = ShallowMortonCode;
			using MortonPtr = shared_ptr< Morton >;
			using MortonPtrInternals = PtrInternals< Morton, ManagedAllocator< Morton > >;
			
			using Point = model::Point;
			using PointPtr = shared_ptr< Point >;
			using PointPtrInternals = PtrInternals< Point, ManagedAllocator< Point > >;
			using PointVector = vector< PointPtr, ManagedAllocator< PointPtr > >;
			
			using Node = OctreeNode< PointVector >;
			using NodePtr = shared_ptr< Node >;
			using NodePtrInternals = PtrInternals< Node, ManagedAllocator< Node > >;
			
			using OctreeMap = model::OctreeMap< Morton, Node >;
			using MapInternals = model::MapInternals< Morton, Node >;
			
			uint nNodes = 500000u;
			uint nPoints = 2u * nNodes;
			size_t maxMemToUse = nNodes * ( sizeof( MortonPtrInternals ) + sizeof( NodePtrInternals )
								+ sizeof( MapInternals ) + 3 * ( sizeof( PointPtr ) ) )
								+ nPoints * sizeof( PointPtrInternals );
			
			initManager< Morton, Point, Node >( maxMemToUse, TypeParam() );
			IMemoryManager& manager = SingletonMemoryManager::instance();
			
			ASSERT_EQ( 0, manager.usedMemory() );
			
			OctreeMap map;
			
			for( unsigned int i = 0u; i < nNodes; ++i )
			{
				MortonPtr mortonCode = makeManaged< Morton >();
				mortonCode->build( i );
				
				NodePtr node = makeManaged< Node >( true );
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
			
			using Node = OctreeNode< PointVector >;
			using NodePtr = shared_ptr< Node >;
			using NodePtrInternals = PtrInternals< Node, ManagedAllocator< Node > >;
			
			using OctreeMap = model::OctreeMap< Morton, Node >;
			using MapInternals = model::MapInternals< Morton, Node >;
			
			uint nNodes = 500000u;
			uint nPoints = 2u * nNodes;
			size_t maxMemToUse = nNodes * ( sizeof( MortonPtrInternals ) + sizeof( NodePtrInternals )
								+ sizeof( MapInternals ) + 3 * ( sizeof( PointPtr ) ) )
								+ nPoints * sizeof( PointPtrInternals );
			
			initManager< Morton, Point, Node >( maxMemToUse, TypeParam() );
			IMemoryManager& manager = SingletonMemoryManager::instance();
			
			ASSERT_EQ( 0, manager.usedMemory() );
			
			OctreeMap map;
			
			for( unsigned int i = 0u; i < nNodes; ++i )
			{
				MortonPtr mortonCode = makeManaged< Morton >();
				mortonCode->build( i );
				
				NodePtr node = makeManaged< Node >( true );
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
		
		TYPED_TEST( MemoryManagerTest, SimpleArrays )
		{
			using Morton = ShallowMortonCode;
			using Point = model::Point;
			using PointPtr = shared_ptr< Point >;
			using PointVector = vector< PointPtr, ManagedAllocator< PointPtr > >;
			using Node = OctreeNode< PointVector >;
			using NodeVector = vector< Node >;
			
			uint arraySizes[ 5 ] = { 100000u, 150000u, 50000u, 70000u, 120000u };
			
			initManager< Morton, Point, Node >( 1024 * 1024, TypeParam() );
			
			Morton* mortonArrays[ 5 ];
			Point* pointArrays[ 5 ];
			NodeVector nodeArrays[ 5 ];
			
			for( int i = 0; i < 5; ++i )
			{
				mortonArrays[ i ] = new Morton[ arraySizes[ i ] ];
				pointArrays[ i ] = new Point[ arraySizes[ i ] ];
				nodeArrays[ i ] = NodeVector( arraySizes[ i ], Node( true ) );
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
					ASSERT_EQ( nodeArrays[ i ].data() - nodeArrays[ i - 1 ].data(), arraySizes[ i - 1 ] + 1 );
				}
			}
			
			// After validation, the acquired pointers are set as the expected value for allocation return.
			Morton* expectedMortonArrays[ 5 ];
			Point* expectedPointArrays[ 5 ];
			Node* expectedNodeArrays[ 5 ];
			
			for( int i = 0; i < 5; ++i )
			{
				expectedMortonArrays[ i ] = mortonArrays[ i ];
				expectedPointArrays[ i ] = pointArrays[ i ];
				expectedNodeArrays[ i ] = nodeArrays[ i ].data();
			}
			
			// Deleting some arrays in order to check if remaining pointers are correct afterwards.
			for( int i = 0; i < 3; ++i )
			{
				delete[] mortonArrays[ i ];
				delete[] pointArrays[ i ];
				nodeArrays[ i ] = NodeVector();
				
				// Dereferencing attempt to check integrity after deletes.
				for( int j = i + 1; j < 5; ++j )
				{
					ASSERT_NO_THROW( Morton morton = mortonArrays[ j ][ 0 ] );
					ASSERT_NO_THROW( Point point = pointArrays[ j ][ 0 ] );
					ASSERT_NO_THROW( Node node = nodeArrays[ j ][ 0 ] );
				}
			}
			
			// Creating arrays after deletions in order to check if memory is handled correctly after deletions.
			for( int i = 0; i < 3; ++i )
			{
				mortonArrays[ i ] = new Morton[ arraySizes[ i ] ];
				pointArrays[ i ] = new Point[ arraySizes[ i ] ];
				nodeArrays[ i ] = NodeVector( arraySizes[ i ], Node( true ) );
				
				int modPrev = ( ( i - 1 ) % 5 + 5 ) % 5;
				
				ASSERT_EQ( mortonArrays[ i ] - mortonArrays[ modPrev  ] - arraySizes[ modPrev ] - 1, 0 );
				ASSERT_EQ( pointArrays[ i ] - pointArrays[ modPrev ] - arraySizes[ modPrev ] - 1, 0 );
				ASSERT_EQ( nodeArrays[ i ].data() - nodeArrays[ modPrev ].data() - arraySizes[ modPrev ] - 1, 0 );
			}
			
			// Deleting all arrays to setup the case where the pool could be lost because all ArrayMemoryInfo are erased.
			for( int i = 0; i < 5; ++i )
			{
				delete[] mortonArrays[ i ];
				delete[] pointArrays[ i ];
				nodeArrays[ i ] = NodeVector();
			}
			
			// Alloc again to check if memory is reused after deletion.
			for( int i = 0; i < 5; ++i )
			{
				mortonArrays[ i ] = new Morton[ arraySizes[ i ] ];
				pointArrays[ i ] = new Point[ arraySizes[ i ] ];
				nodeArrays[ i ] = NodeVector( arraySizes[ i ], Node( true ) );
				
				ASSERT_EQ( mortonArrays[ i ] - expectedMortonArrays[ i ], 0 );
				ASSERT_EQ( pointArrays[ i ] - expectedPointArrays[ i ], 0 );
				ASSERT_EQ( nodeArrays[ i ].data() - expectedNodeArrays[ i ], 0 );
			}
			
			for( int i = 0; i < 5; ++i )
			{
				delete[] mortonArrays[ i ];
				delete[] pointArrays[ i ];
			}
		}
		
		TYPED_TEST( MemoryManagerTest, ComplexArrays )
		{
			using Morton = MediumMortonCode;
			using Point = model::ExtendedPoint;
			using PointPtr = shared_ptr< Point >;
			using PointVector = vector< PointPtr, ManagedAllocator< PointPtr > >;
			using Node = OctreeNode< PointVector >;
			using NodeVector = vector< Node >;
			
			uint arraySizes[ 5 ] = { 100000u, 150000u, 50000u, 70000u, 120000u };
			
			initManager< Morton, Point, Node >( 1024 * 1024, TypeParam() );
			
			Morton* mortonArrays[ 5 ];
			Point* pointArrays[ 5 ];
			NodeVector nodeArrays[ 5 ];
			
			for( int i = 0; i < 5; ++i )
			{
				mortonArrays[ i ] = new Morton[ arraySizes[ i ] ];
				pointArrays[ i ] = new Point[ arraySizes[ i ] ];
				nodeArrays[ i ] = NodeVector( arraySizes[ i ], true );
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
					ASSERT_EQ( nodeArrays[ i ].data() - nodeArrays[ i - 1 ].data(), arraySizes[ i - 1 ] + 1 );
				}
			}
			
			// After validation, the acquired pointers are set as the expected value for allocation return.
			Morton* expectedMortonArrays[ 5 ];
			Point* expectedPointArrays[ 5 ];
			Node* expectedNodeArrays[ 5 ];
			
			for( int i = 0; i < 5; ++i )
			{
				expectedMortonArrays[ i ] = mortonArrays[ i ];
				expectedPointArrays[ i ] = pointArrays[ i ];
				expectedNodeArrays[ i ] = nodeArrays[ i ].data();
			}
			
			// Deleting some arrays in order to check if remaining pointers are correct afterwards.
			for( int i = 0; i < 3; ++i )
			{
				delete[] mortonArrays[ i ];
				delete[] pointArrays[ i ];
				nodeArrays[ i ] = NodeVector();
				
				// Dereferencing attempt to check integrity after deletes.
				for( int j = i + 1; j < 5; ++j )
				{
					ASSERT_NO_THROW( Morton morton = mortonArrays[ j ][ 0 ] );
					ASSERT_NO_THROW( Point point = pointArrays[ j ][ 0 ] );
					ASSERT_NO_THROW( Node point = nodeArrays[ j ][ 0 ] );
				}
			}
			
			// Creating arrays after deletions in order to check if memory is handled correctly after deletions.
			for( int i = 0; i < 3; ++i )
			{
				mortonArrays[ i ] = new Morton[ arraySizes[ i ] ];
				pointArrays[ i ] = new Point[ arraySizes[ i ] ];
				nodeArrays[ i ] = NodeVector( arraySizes[ i ], Node( true ) );
				
				int modPrev = ( ( i - 1 ) % 5 + 5 ) % 5;
				
				ASSERT_EQ( mortonArrays[ i ] - mortonArrays[ modPrev  ] - arraySizes[ modPrev ] - 1, 0 );
				ASSERT_EQ( pointArrays[ i ] - pointArrays[ modPrev ] - arraySizes[ modPrev ] - 1, 0 );
				ASSERT_EQ( nodeArrays[ i ].data() - nodeArrays[ modPrev ].data() - arraySizes[ modPrev ] - 1, 0 );
			}
			
			// Deleting all arrays to setup the case where the pool could be lost because all ArrayMemoryInfo are erased.
			for( int i = 0; i < 5; ++i )
			{
				delete[] mortonArrays[ i ];
				delete[] pointArrays[ i ];
				nodeArrays[ i ] = NodeVector();
			}
			
			// Alloc again to check if memory is reused after deletion.
			for( int i = 0; i < 5; ++i )
			{
				mortonArrays[ i ] = new Morton[ arraySizes[ i ] ];
				pointArrays[ i ] = new Point[ arraySizes[ i ] ];
				nodeArrays[ i ] = NodeVector( arraySizes[ i ], Node( true ) );
				
				ASSERT_EQ( mortonArrays[ i ] - expectedMortonArrays[ i ], 0 );
				ASSERT_EQ( pointArrays[ i ] - expectedPointArrays[ i ], 0 );
				ASSERT_EQ( nodeArrays[ i ].data() - expectedNodeArrays[ i ], 0 );
			}
			
			for( int i = 0; i < 5; ++i )
			{
				delete[] mortonArrays[ i ];
				delete[] pointArrays[ i ];
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
			
			using Node = OctreeNode< PointVector >;
			using NodePtr = shared_ptr< Node >;
			using NodePtrInternals = PtrInternals< Node, ManagedAllocator< Node > >;
			
			using OctreeMap = model::OctreeMap< Morton, Node >;
			using MapInternals = model::MapInternals< Morton, Node >;
			
			int nPoints = 10000;
			size_t expectedMemUsage = 	nPoints * ( 3 * sizeof( PointPtr ) + sizeof( PointPtrInternals ) )
										+ 3 * sizeof( PointPtr ) + 2 * sizeof( MortonPtrInternals )
										+ 2 * sizeof( NodePtrInternals ) + 2 * sizeof( MapInternals );
			
			//BitMapMemoryManager< Morton, Point, Inner, Leaf >::initInstance( expectedMemUsage );
			initManager< Morton, Point, Node >( expectedMemUsage, TypeParam() );
			
			PointVector points( nPoints );
			for( int i = 0; i < nPoints; ++i )
			{
				PointPtr point = makeManaged< Point >();
				points[ i ] = point;
			}
			
			NodePtr inner = makeManaged< Node >( false );
			inner->setContents( points );
			
			NodePtr leaf = makeManaged< Node >( true );
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
			
			using Node = OctreeNode< IndexVector >;
			using NodePtr = shared_ptr< Node >;
			using NodePtrInternals = PtrInternals< Node, ManagedAllocator< Node > >;
			
			using OctreeMap = model::OctreeMap< Morton, Node >;
			using MapInternals = model::MapInternals< Morton, Node >;
			
			int nPoints = 10000;
			size_t expectedMemUsage = 	nPoints * ( sizeof( PointPtr ) + 3 * sizeof( Index ) + sizeof( PointPtrInternals ) )
										+ sizeof( PointPtr ) + 3 * sizeof( Index ) + 2 * sizeof( MortonPtrInternals )
										+ 2 * sizeof( NodePtrInternals ) + 2 * sizeof( MapInternals );
			
			initManager< Morton, Point, Node >( expectedMemUsage, TypeParam() );
			
			PointVector points( nPoints );
			IndexVector indices( nPoints );
			for( int i = 0; i < nPoints; ++i )
			{
				points[ i ] = makeManaged< Point >();
				indices[ i ] = i;
			}
			
			NodePtr inner = makeManaged< Node >( false );
			inner->setContents( indices );
			
			NodePtr leaf = makeManaged< Node >( true );
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