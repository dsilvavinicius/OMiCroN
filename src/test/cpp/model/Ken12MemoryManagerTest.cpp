#include <gtest/gtest.h>
#include "MemoryManager.h"
#include "OctreeMapTypes.h"
#include "MortonCode.h"
#include "OctreeNode.h"
#include "Ken12MemoryManager.h"

using namespace std;

namespace model
{
	namespace test
	{
		/*
        class Ken12MemoryManagerTest : public ::testing::Test
		{};
		
		TEST_F( Ken12MemoryManagerTest, ShallowPointVectorLeafNodes )
		{
			using Morton = ShallowMortonCode;
			using MortonPtr = shared_ptr< Morton >;
			using Point = model::Point;
			using PointPtr = shared_ptr< Point >;
			using PointVector = vector< PointPtr >;
			using Node = OctreeNode< PointVector >;
			using NodePtr = shared_ptr< Node >;
			using OctreeMap = model::DefaultOctreeMap< Morton, Node >;
			
			size_t nNodes = 500000u;
			size_t nPoints = 2u * nNodes;
			
			Ken12MemoryManager< Morton, Point, Node >::initInstance( nNodes, nPoints, nNodes );
			
			IMemoryManager& manager = SingletonMemoryManager::instance();
			
			ASSERT_EQ( manager.usedMemory(), ( size_t ) 0 );
			
			OctreeMap map;
			
			for( unsigned int i = 0u; i < nNodes; ++i )
			{
				if( i == 0.5 * nNodes )
				{
					size_t expectedHalfSize = 	0.5 * nNodes * sizeof( Morton ) + 0.5 * nPoints * sizeof( Point )
												+ 0.5 * nNodes * sizeof( Node );
					ASSERT_EQ( manager.usedMemory(), expectedHalfSize );
				}
				
				MortonPtr mortonCode( new Morton() );
				mortonCode->build( i );
				
				NodePtr node( new Node( true ) );
				PointPtr p0( new Point() );
				PointPtr p1( new Point() );
				node->setContents( PointVector( { p0, p1 } ) );
				
				map[ mortonCode ] = node;
			}
			
			size_t expectedFullSize = nNodes * sizeof( Morton ) + nPoints * sizeof( Point ) + nNodes * sizeof( Node );
			ASSERT_EQ( manager.usedMemory(), expectedFullSize );
			
			map.clear();
			
			ASSERT_EQ( manager.usedMemory(), ( size_t ) 0 );
		}
		
		TEST_F( Ken12MemoryManagerTest, MediumExtendedPointVectorInnerNodes )
		{
			using Morton = MediumMortonCode;
			using MortonPtr = shared_ptr< Morton >;
			using Point = model::ExtendedPoint;
			using PointPtr = shared_ptr< Point >;
			using PointVector = vector< PointPtr >;
			using Node = OctreeNode< PointVector >;
			using NodePtr = shared_ptr< Node >;
			using OctreeMap = model::DefaultOctreeMap< Morton, Node >;
			
			size_t nNodes = 500000u;
			size_t nPoints = 2u * nNodes;
			
			Ken12MemoryManager< Morton, Point, Node >::initInstance( nNodes, nPoints, nNodes );
			
			IMemoryManager& manager = SingletonMemoryManager::instance();
			
			ASSERT_EQ( manager.usedMemory(), ( size_t ) 0 );
			
			OctreeMap map;
			
			for( unsigned int i = 0u; i < nNodes; ++i )
			{
				if( i == 0.5 * nNodes )
				{
					size_t expectedHalfSize = 	0.5 * nNodes * sizeof( Morton ) + 0.5 * nPoints * sizeof( Point )
												+ 0.5 * nNodes * sizeof( Node );
					ASSERT_EQ( manager.usedMemory(), expectedHalfSize );
				}
				
				MortonPtr mortonCode( new Morton() );
				mortonCode->build( i );
				
				NodePtr node( new Node( true ) );
				PointPtr p0( new Point() );
				PointPtr p1( new Point() );
				node->setContents( PointVector( { p0, p1 } ) );
				
				map[ mortonCode ] = node;
			}
			
			size_t expectedFullSize = 	nNodes * sizeof( Morton ) + nPoints * sizeof( Point ) + nNodes * sizeof( Node );
			ASSERT_EQ( manager.usedMemory(), expectedFullSize );
			
			map.clear();
			
			ASSERT_EQ( manager.usedMemory(), ( size_t ) 0 );
		}*/
	}
}