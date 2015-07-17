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
			ASSERT_EQ( MemoryManager::instance().availableShallowMortonBlocks(), 20709060u );
			ASSERT_EQ( MemoryManager::instance().availableMediumMortonBlocks(), 20709060u );
			ASSERT_EQ( MemoryManager::instance().availablePointBlocks(), 41418120u );
			ASSERT_EQ( MemoryManager::instance().availableExtendedPointBlocks(), 41418120u );
			ASSERT_EQ( MemoryManager::instance().availableNodeBlocks(), 20709060u );
			
			ShallowOctreeMap map;
			
			for( unsigned int i = 0u; i < 20709060u; ++i )
			{
				ShallowMortonCodePtr mortonCode( new ShallowMortonCode() );
				mortonCode->build( i );
				
				ShallowLeafNodePtr< PointVector > node( new ShallowLeafNode< PointVector >() );
				PointPtr p0( new Point() );
				PointPtr p1( new Point() );
				node->setContents( PointVector( { p0, p1 } ) );
				
				map[ mortonCode ] = node;
			}
			
			ASSERT_EQ( MemoryManager::instance().availableShallowMortonBlocks(), 0u );
			ASSERT_EQ( MemoryManager::instance().availableMediumMortonBlocks(), 20709060u );
			ASSERT_EQ( MemoryManager::instance().availablePointBlocks(), 0u );
			ASSERT_EQ( MemoryManager::instance().availableExtendedPointBlocks(), 41418120u );
			ASSERT_EQ( MemoryManager::instance().availableNodeBlocks(), 0u );
				 
			map.clear();
			
			ASSERT_EQ( MemoryManager::instance().availableShallowMortonBlocks(), 20709060u );
			ASSERT_EQ( MemoryManager::instance().availableMediumMortonBlocks(), 20709060u );
			ASSERT_EQ( MemoryManager::instance().availablePointBlocks(), 41418120u );
			ASSERT_EQ( MemoryManager::instance().availableExtendedPointBlocks(), 41418120u );
			ASSERT_EQ( MemoryManager::instance().availableNodeBlocks(), 20709060u );
		}
		
		TEST_F( MemoryManagerTest, MediumExtendedPointVectorInnerNodes )
		{
			ASSERT_EQ( MemoryManager::instance().availableShallowMortonBlocks(), 20709060u );
			ASSERT_EQ( MemoryManager::instance().availableMediumMortonBlocks(), 20709060u );
			ASSERT_EQ( MemoryManager::instance().availablePointBlocks(), 41418120u );
			ASSERT_EQ( MemoryManager::instance().availableExtendedPointBlocks(), 41418120u );
			ASSERT_EQ( MemoryManager::instance().availableNodeBlocks(), 20709060u );
			
			MediumOctreeMap map;
			
			for( unsigned int i = 0u; i < 20709060u; ++i )
			{
				MediumMortonCodePtr mortonCode( new MediumMortonCode() );
				mortonCode->build( i );
				
				MediumLeafNodePtr< ExtendedPointVector > node( new MediumLeafNode< ExtendedPointVector >() );
				ExtendedPointPtr p0( new ExtendedPoint() );
				ExtendedPointPtr p1( new ExtendedPoint() );
				node->setContents( ExtendedPointVector( { p0, p1 } ) );
				
				map[ mortonCode ] = node;
			}
			
			ASSERT_EQ( MemoryManager::instance().availableShallowMortonBlocks(), 20709060u );
			ASSERT_EQ( MemoryManager::instance().availableMediumMortonBlocks(), 0 );
			ASSERT_EQ( MemoryManager::instance().availablePointBlocks(), 41418120u );
			ASSERT_EQ( MemoryManager::instance().availableExtendedPointBlocks(), 0 );
			ASSERT_EQ( MemoryManager::instance().availableNodeBlocks(), 0 );
				 
			map.clear();
			
			ASSERT_EQ( MemoryManager::instance().availableShallowMortonBlocks(), 20709060u );
			ASSERT_EQ( MemoryManager::instance().availableMediumMortonBlocks(), 20709060u );
			ASSERT_EQ( MemoryManager::instance().availablePointBlocks(), 41418120u );
			ASSERT_EQ( MemoryManager::instance().availableExtendedPointBlocks(), 41418120u );
			ASSERT_EQ( MemoryManager::instance().availableNodeBlocks(), 20709060u );
		}
	}
}