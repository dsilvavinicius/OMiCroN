#include <gtest/gtest.h>
#include "MemoryManager.h"
#include "OctreeMapTypes.h"
#include "MortonCode.h"
#include "InnerNode.h"
#include "LeafNode.h"
#include <Ken12MemoryManager.h>

using namespace std;

namespace model
{
	namespace test
	{
        class MemoryManagerTest : public ::testing::Test
		{};
		
		TEST_F( MemoryManagerTest, ShallowPointVectorLeafNodes )
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
			using OctreeMap = model::DefaultOctreeMap< Morton >;
			
			size_t nNodes = 500000u;
			size_t nPoints = 2u * nNodes;
			
			Ken12MemoryManager< Morton, Point, Inner, Leaf >::initInstance( nNodes, nPoints, 0, nNodes );
			
			IMemoryManager& manager = SingletonMemoryManager::instance();
			
			ASSERT_EQ( manager.usedMemory(), ( size_t ) 0 );
			
			OctreeMap map;
			
			for( unsigned int i = 0u; i < nNodes; ++i )
			{
				if( i == 0.5 * nNodes )
				{
					size_t expectedHalfSize = 	0.5 * nNodes * sizeof( Morton ) + 0.5 * nPoints * sizeof( Point )
												+ 0.5 * nNodes * sizeof( Leaf );
					ASSERT_EQ( manager.usedMemory(), expectedHalfSize );
				}
				
				MortonPtr mortonCode( new Morton() );
				mortonCode->build( i );
				
				LeafPtr node( new Leaf() );
				PointPtr p0( new Point() );
				PointPtr p1( new Point() );
				node->setContents( PointVector( { p0, p1 } ) );
				
				map[ mortonCode ] = node;
			}
			
			size_t expectedFullSize = nNodes * sizeof( Morton ) + nPoints * sizeof( Point ) + nNodes * sizeof( Leaf );
			ASSERT_EQ( manager.usedMemory(), expectedFullSize );
			
			map.clear();
			
			ASSERT_EQ( manager.usedMemory(), ( size_t ) 0 );
		}
		
		TEST_F( MemoryManagerTest, MediumExtendedPointVectorInnerNodes )
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
			using OctreeMap = model::DefaultOctreeMap< Morton >;
			
			size_t nNodes = 500000u;
			size_t nPoints = 2u * nNodes;
			
			Ken12MemoryManager< Morton, Point, Inner, Leaf >::initInstance( nNodes, nPoints, 0, nNodes );
			
			IMemoryManager& manager = SingletonMemoryManager::instance();
			
			ASSERT_EQ( manager.usedMemory(), ( size_t ) 0 );
			
			OctreeMap map;
			
			for( unsigned int i = 0u; i < nNodes; ++i )
			{
				if( i == 0.5 * nNodes )
				{
					size_t expectedHalfSize = 	0.5 * nNodes * sizeof( Morton ) + 0.5 * nPoints * sizeof( Point )
												+ 0.5 * nNodes * sizeof( Leaf );
					ASSERT_EQ( manager.usedMemory(), expectedHalfSize );
				}
				
				MortonPtr mortonCode( new Morton() );
				mortonCode->build( i );
				
				LeafPtr node( new Leaf() );
				PointPtr p0( new Point() );
				PointPtr p1( new Point() );
				node->setContents( PointVector( { p0, p1 } ) );
				
				map[ mortonCode ] = node;
			}
			
			size_t expectedFullSize = 	nNodes * sizeof( Morton ) + nPoints * sizeof( Point ) + nNodes * sizeof( Leaf );
			ASSERT_EQ( manager.usedMemory(), expectedFullSize );
			
			map.clear();
			
			ASSERT_EQ( manager.usedMemory(), ( size_t ) 0 );
		}
	}
}