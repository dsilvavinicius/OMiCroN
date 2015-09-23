#include <gtest/gtest.h>
#include "BitMapMemoryManager.h"
#include "OctreeMapTypes.h"
#include <InnerNode.h>

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
			using Inner = ShallowInnerNode< PointVector >;
			using InnerPtr = shared_ptr< Inner >;
			using Leaf = ShallowLeafNode< PointVector >;
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
				MortonPtr mortonCode( new Morton() );
				mortonCode->build( i );
				
				LeafPtr node( new Leaf() );
				PointPtr p0( new Point() );
				PointPtr p1( new Point() );
				node->setContents( PointVector( { p0, p1 } ) );
				
				map[ mortonCode ] = node;
			}
			
			ASSERT_EQ( maxMemToUse, manager.usedMemory() );
			
			for( unsigned int i = 0u; i < nNodes; ++i )
			{
				MortonPtr expected( new Morton() );
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
			using Inner = MediumInnerNode< PointVector >;
			using InnerPtr = shared_ptr< Inner >;
			using Leaf = MediumLeafNode< PointVector >;
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
				MortonPtr mortonCode( new Morton() );
				mortonCode->build( i );
				
				InnerPtr node( new Inner() );
				PointPtr p0( new Point() );
				PointPtr p1( new Point() );
				node->setContents( PointVector( { p0, p1 } ) );
				
				map[ mortonCode ] = node;
			}
			
			ASSERT_EQ( maxMemToUse, manager.usedMemory() );
			
			for( unsigned int i = 0u; i < nNodes; ++i )
			{
				MortonPtr expected( new Morton() );
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
			using Inner = MediumInnerNode< ExtendedPointPtr >;
			using InnerPtr = shared_ptr< Inner >;
			using Leaf = MediumLeafNode< PointVector >;
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
				MortonPtr mortonCode( new Morton() );
				mortonCode->build( i );
				
				InnerPtr node( new Inner() );
				PointPtr point( new Point() );
				node->setContents( point );
				
				map[ mortonCode ] = node;
			}
			
			ASSERT_EQ( maxMemToUse, manager.usedMemory() );
			
			for( unsigned int i = 0u; i < nNodes; ++i )
			{
				MortonPtr expected( new Morton() );
				expected->build( i );
				
				ASSERT_TRUE( map.find( expected ) != map.end() );
			}
			
			map.clear();
			
			ASSERT_EQ( 0, manager.usedMemory() );
		}
	}
}