#include <gtest/gtest.h>
#include "BitMapMemoryManager.h"

namespace model
{
	namespace test
	{
        class BitMapMemoryManagerTest : public ::testing::Test
		{};
		
		TEST_F( BitMapMemoryManagerTest, ManagedTypes )
		{
			// Deactivating default manager
			MemoryManager::initInstance( 0, 0, 0, 0, 0 );
			
			uint nNodes = 500000u;
			uint nPoints = 2u * nNodes;
			size_t maxMemToUse = nNodes * sizeof( ShallowMortonCode ) + nPoints * sizeof( Point )
									+ nNodes * sizeof( ShallowLeafNodePtr< PointVector > );
			BitMapMemoryManager::initInstance( maxMemToUse );
			
			ASSERT_EQ( 0, manager.usedMemory() );
			
			BitMapMemoryManager& manager = dynamic_cast< BitMapMemoryManager& >( BitMapMemoryManager::instance() );
			
			ShallowOctreeMap map;
			
			for( unsigned int i = 0u; i < nNodes; ++i )
			{
				ShallowMortonCodePtr mortonCode( new ShallowMortonCode() );
				mortonCode->build( i );
				
				ShallowLeafNodePtr< PointVector > node( new ShallowLeafNode< PointVector >() );
				PointPtr p0( new Point() );
				PointPtr p1( new Point() );
				node->setContents( PointVector( { p0, p1 } ) );
				
				map[ mortonCode ] = node;
			}
			
			ASSERT_EQ( maxMemToUse, manager.usedMemory() );
			
			map.clear();
			
			ASSERT_EQ( 0, manager.usedMemory() );
		}
	}
}