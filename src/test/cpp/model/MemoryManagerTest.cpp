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
			
			/*initManager< Morton, Point, Node >( maxMemToUse, TypeParam() );
			IMemoryManager& manager = SingletonMemoryManager::instance();
			
			ASSERT_EQ( 0, manager.usedMemory() );*/
			
			ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
			
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
			
			//ASSERT_EQ( maxMemToUse, manager.usedMemory() );
			ASSERT_EQ( maxMemToUse, AllocStatistics::totalAllocated() );
			
			for( unsigned int i = 0u; i < nNodes; ++i )
			{
				MortonPtr expected = makeManaged< Morton >();
				expected->build( i );
				
				ASSERT_TRUE( map.find( expected ) != map.end() );
			}
			
			map.clear();
			
			//ASSERT_EQ( 0, manager.usedMemory() );
			ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
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
			
			/*initManager< Morton, Point, Node >( maxMemToUse, TypeParam() );
			IMemoryManager& manager = SingletonMemoryManager::instance();*/
			
			//ASSERT_EQ( 0, manager.usedMemory() );
			ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
			
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
			
			//ASSERT_EQ( maxMemToUse, manager.usedMemory() );
			ASSERT_EQ( maxMemToUse, AllocStatistics::totalAllocated() );
			
			for( unsigned int i = 0u; i < nNodes; ++i )
			{
				MortonPtr expected = makeManaged< Morton >();
				expected->build( i );
				
				ASSERT_TRUE( map.find( expected ) != map.end() );
			}
			
			map.clear();
			
			//ASSERT_EQ( 0, manager.usedMemory() );
			ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
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
			//initManager< Morton, Point, Node >( expectedMemUsage, TypeParam() );
			
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
			
			ASSERT_EQ( expectedMemUsage, AllocStatistics::totalAllocated() );
			//ASSERT_EQ( SingletonMemoryManager::instance().usedMemory(), expectedMemUsage );
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
			
			//initManager< Morton, Point, Node >( expectedMemUsage, TypeParam() );
			
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
			
			ASSERT_EQ( expectedMemUsage, AllocStatistics::totalAllocated() );
			//ASSERT_EQ( SingletonMemoryManager::instance().usedMemory(), expectedMemUsage );
		}
	}
}