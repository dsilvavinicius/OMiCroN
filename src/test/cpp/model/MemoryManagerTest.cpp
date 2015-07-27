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
			uint nNodes = 500000u;
			uint nPoints = 2u * nNodes;
			
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::FOUR_BYTES ), nNodes );
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::EIGHT_BYTES ), nNodes );
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::TWENTY_FOUR_BYTES ), nPoints );
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::THIRTY_SIX ), nPoints );
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::THIRTY_TWO ), nNodes );
			
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::FOUR_BYTES ), 1.f );
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::EIGHT_BYTES ), 1.f );
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::TWENTY_FOUR_BYTES ), 1.f );
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::THIRTY_SIX ), 1.f );
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::THIRTY_TWO ), 1.f );
			
			ShallowOctreeMap map;
			
			for( unsigned int i = 0u; i < nNodes; ++i )
			{
				if( i == 0.5 * nNodes )
				{
					ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::FOUR_BYTES ), 0.5 * nNodes );
					ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::EIGHT_BYTES ), nNodes );
					ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::TWENTY_FOUR_BYTES ), 0.5 * nPoints );
					ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::THIRTY_SIX ), nPoints );
					ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::THIRTY_TWO ), 0.5 * nNodes );
					
					ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::FOUR_BYTES ), 0.5f );
					ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::EIGHT_BYTES ), 1.f );
					ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::TWENTY_FOUR_BYTES ), 0.5f );
					ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::THIRTY_SIX ), 1.f );
					ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::THIRTY_TWO ), 0.5f );
				}
				
				ShallowMortonCodePtr mortonCode( new ShallowMortonCode() );
				mortonCode->build( i );
				
				ShallowLeafNodePtr< PointVector > node( new ShallowLeafNode< PointVector >() );
				PointPtr p0( new Point() );
				PointPtr p1( new Point() );
				node->setContents( PointVector( { p0, p1 } ) );
				
				map[ mortonCode ] = node;
			}
			
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::FOUR_BYTES ), 0u );
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::EIGHT_BYTES ), nNodes );
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::TWENTY_FOUR_BYTES ), 0u );
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::THIRTY_SIX ), nPoints );
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::THIRTY_TWO ), 0u );
			
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::FOUR_BYTES ), 0.f );
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::EIGHT_BYTES ), 1.f );
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::TWENTY_FOUR_BYTES ), 0.f );
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::THIRTY_SIX ), 1.f );
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::THIRTY_TWO ), 0.f );
			
			map.clear();
			
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::FOUR_BYTES ), nNodes );
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::EIGHT_BYTES ), nNodes );
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::TWENTY_FOUR_BYTES ), nPoints );
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::THIRTY_SIX ), nPoints );
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::THIRTY_TWO ), nNodes );
			
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::FOUR_BYTES ), 1.f );
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::EIGHT_BYTES ), 1.f );
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::TWENTY_FOUR_BYTES ), 1.f );
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::THIRTY_SIX ), 1.f );
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::THIRTY_TWO ), 1.f );
		}
		
		TEST_F( MemoryManagerTest, MediumExtendedPointVectorInnerNodes )
		{
			uint nNodes = 500000u;
			uint nPoints = 2u * nNodes;
			
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::FOUR_BYTES ), nNodes );
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::EIGHT_BYTES ), nNodes );
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::TWENTY_FOUR_BYTES ), nPoints );
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::THIRTY_SIX ), nPoints );
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::THIRTY_TWO ), nNodes );
			
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::FOUR_BYTES ), 1.f );
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::EIGHT_BYTES ), 1.f );
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::TWENTY_FOUR_BYTES ), 1.f );
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::THIRTY_SIX ), 1.f );
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::THIRTY_TWO ), 1.f );
			
			MediumOctreeMap map;
			
			for( unsigned int i = 0u; i < nNodes; ++i )
			{
				if( i == 0.5 * nNodes )
				{
					ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::FOUR_BYTES ), nNodes );
					ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::EIGHT_BYTES ), 0.5 * nNodes );
					ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::TWENTY_FOUR_BYTES ), nPoints );
					ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::THIRTY_SIX ), 0.5 * nPoints );
					ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::THIRTY_TWO ), 0.5 * nNodes );
					
					ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::FOUR_BYTES ), 1.f );
					ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::EIGHT_BYTES ), 0.5f );
					ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::TWENTY_FOUR_BYTES ), 1.f );
					ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::THIRTY_SIX ), 0.5f );
					ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::THIRTY_TWO ), 0.5f );
				}
				
				MediumMortonCodePtr mortonCode( new MediumMortonCode() );
				mortonCode->build( i );
				
				MediumLeafNodePtr< ExtendedPointVector > node( new MediumLeafNode< ExtendedPointVector >() );
				ExtendedPointPtr p0( new ExtendedPoint() );
				ExtendedPointPtr p1( new ExtendedPoint() );
				node->setContents( ExtendedPointVector( { p0, p1 } ) );
				
				map[ mortonCode ] = node;
			}
			
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::FOUR_BYTES ), nNodes );
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::EIGHT_BYTES ), 0u );
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::TWENTY_FOUR_BYTES ), nPoints );
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::THIRTY_SIX ), 0u );
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::THIRTY_TWO ), 0u );
			
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::FOUR_BYTES ), 1.f );
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::EIGHT_BYTES ), 0.f );
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::TWENTY_FOUR_BYTES ), 1.f );
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::THIRTY_SIX ), 0.f );
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::THIRTY_TWO ), 0.f );
			
			map.clear();
			
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::FOUR_BYTES ), nNodes );
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::EIGHT_BYTES ), nNodes );
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::TWENTY_FOUR_BYTES ), nPoints );
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::THIRTY_SIX ), nPoints );
			ASSERT_EQ( MemoryManager::instance().freeBlocks( MemoryManager::THIRTY_TWO ), nNodes );
			
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::FOUR_BYTES ), 1.f );
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::EIGHT_BYTES ), 1.f );
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::TWENTY_FOUR_BYTES ), 1.f );
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::THIRTY_SIX ), 1.f );
			ASSERT_EQ( MemoryManager::instance().freeBlocksPercentage( MemoryManager::THIRTY_TWO ), 1.f );
		}
	}
}