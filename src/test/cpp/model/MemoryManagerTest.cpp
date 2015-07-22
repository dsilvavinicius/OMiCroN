#include <gtest/gtest.h>
#include "MemoryManager.h"
#include "OctreeMapTypes.h"
#include "MortonCode.h"
#include "LeafNode.h"

using namespace std;

namespace model
{
	namespace test
	{
        class MemoryManagerTest : public ::testing::Test
		{};
		
		TEST_F( MemoryManagerTest, ShallowPointVectorLeafNodes )
		{
			uint nNodes = 20709060u;
			uint nPoints = 2 * 20709060u;
			
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::SHALLOW_MORTON ), nNodes );
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::MEDIUM_MORTON ), nNodes );
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::POINT ), nPoints );
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::EXTENDED_POINT ), nPoints );
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::NODE ), nNodes );
			
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::SHALLOW_MORTON ), 1.f );
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::MEDIUM_MORTON ), 1.f );
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::POINT ), 1.f );
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::EXTENDED_POINT ), 1.f );
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::NODE ), 1.f );
			
			ShallowOctreeMap map;
			
			for( unsigned int i = 0u; i < 20709060u; ++i )
			{
				if( i == 0.5 * 20709060 )
				{
					ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::SHALLOW_MORTON ), 0.5 * nNodes );
					ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::MEDIUM_MORTON ), nNodes );
					ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::POINT ), 0.5 * nPoints );
					ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::EXTENDED_POINT ), nPoints );
					ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::NODE ), 0.5 * nNodes );
					
					ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::SHALLOW_MORTON ), 0.5f );
					ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::MEDIUM_MORTON ), 1.f );
					ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::POINT ), 0.5f );
					ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::EXTENDED_POINT ), 1.f );
					ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::NODE ), 0.5f );
				}
				
				ShallowMortonCodePtr mortonCode( new ShallowMortonCode() );
				mortonCode->build( i );
				
				ShallowLeafNodePtr< PointVector > node( new ShallowLeafNode< PointVector >() );
				PointPtr p0( new Point() );
				PointPtr p1( new Point() );
				node->setContents( PointVector( { p0, p1 } ) );
				
				map[ mortonCode ] = node;
			}
			
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::SHALLOW_MORTON ), 0u );
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::MEDIUM_MORTON ), nNodes );
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::POINT ), 0u );
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::EXTENDED_POINT ), nPoints );
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::NODE ), 0u );
			
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::SHALLOW_MORTON ), 0.f );
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::MEDIUM_MORTON ), 1.f );
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::POINT ), 0.f );
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::EXTENDED_POINT ), 1.f );
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::NODE ), 0.f );
			
			map.clear();
			
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::SHALLOW_MORTON ), nNodes );
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::MEDIUM_MORTON ), nNodes );
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::POINT ), nPoints );
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::EXTENDED_POINT ), nPoints );
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::NODE ), nNodes );
			
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::SHALLOW_MORTON ), 1.f );
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::MEDIUM_MORTON ), 1.f );
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::POINT ), 1.f );
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::EXTENDED_POINT ), 1.f );
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::NODE ), 1.f );
		}
		
		TEST_F( MemoryManagerTest, MediumExtendedPointVectorInnerNodes )
		{
			uint nNodes = 20709060u;
			uint nPoints = 2 * 20709060u;
			
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::SHALLOW_MORTON ), nNodes );
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::MEDIUM_MORTON ), nNodes );
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::POINT ), nPoints );
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::EXTENDED_POINT ), nPoints );
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::NODE ), nNodes );
			
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::SHALLOW_MORTON ), 1.f );
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::MEDIUM_MORTON ), 1.f );
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::POINT ), 1.f );
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::EXTENDED_POINT ), 1.f );
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::NODE ), 1.f );
			
			MediumOctreeMap map;
			
			for( unsigned int i = 0u; i < 20709060u; ++i )
			{
				if( i == 0.5 * 20709060 )
				{
					ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::SHALLOW_MORTON ), nNodes );
					ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::MEDIUM_MORTON ), 0.5 * nNodes );
					ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::POINT ), nPoints );
					ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::EXTENDED_POINT ), 0.5 * nPoints );
					ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::NODE ), 0.5 * nNodes );
					
					ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::SHALLOW_MORTON ), 1.f );
					ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::MEDIUM_MORTON ), 0.5f );
					ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::POINT ), 1.f );
					ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::EXTENDED_POINT ), 0.5f );
					ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::NODE ), 0.5f );
				}
				
				MediumMortonCodePtr mortonCode( new MediumMortonCode() );
				mortonCode->build( i );
				
				MediumLeafNodePtr< ExtendedPointVector > node( new MediumLeafNode< ExtendedPointVector >() );
				ExtendedPointPtr p0( new ExtendedPoint() );
				ExtendedPointPtr p1( new ExtendedPoint() );
				node->setContents( ExtendedPointVector( { p0, p1 } ) );
				
				map[ mortonCode ] = node;
			}
			
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::SHALLOW_MORTON ), nNodes );
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::MEDIUM_MORTON ), 0u );
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::POINT ), nPoints );
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::EXTENDED_POINT ), 0u );
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::NODE ), 0u );
			
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::SHALLOW_MORTON ), 1.f );
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::MEDIUM_MORTON ), 0.f );
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::POINT ), 1.f );
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::EXTENDED_POINT ), 0.f );
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::NODE ), 0.f );
			
			map.clear();
			
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::SHALLOW_MORTON ), nNodes );
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::MEDIUM_MORTON ), nNodes );
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::POINT ), nPoints );
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::EXTENDED_POINT ), nPoints );
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::NODE ), nNodes );
			
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::SHALLOW_MORTON ), 1.f );
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::MEDIUM_MORTON ), 1.f );
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::POINT ), 1.f );
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::EXTENDED_POINT ), 1.f );
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::NODE ), 1.f );
		}
	}
}