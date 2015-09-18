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
		
		TEST_F( BitMapMemoryManagerTest, DISABLED_ManagedTypes0 )
		{
			// Deactivating default manager
			MemoryManager::initInstance( 0, 0, 0, 0, 0 );
			
			uint nNodes = 500000u;
			uint nPoints = 2u * nNodes;
			size_t totalMortonsSize = nNodes * sizeof( ShallowMortonCode );
			size_t totalPointsSize = nPoints * sizeof( Point );
			size_t totalNodesSize = nNodes * sizeof( ShallowLeafNode< PointVector > );
			size_t maxMemToUse = totalMortonsSize + totalPointsSize + totalNodesSize;
			
			BitMapMemoryManager::initInstance( maxMemToUse );
			BitMapMemoryManager& manager = dynamic_cast< BitMapMemoryManager& >( BitMapMemoryManager::instance() );
			
			ASSERT_EQ( 0, manager.usedMemory() );
			
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
		
		TEST_F( BitMapMemoryManagerTest, DISABLED_ManagedTypes1 )
		{
			// Deactivating default manager
			MemoryManager::initInstance( 0, 0, 0, 0, 0 );
			
			uint nNodes = 500000u;
			uint nPoints = 2u * nNodes;
			size_t totalMortonsSize = nNodes * sizeof( MediumMortonCode );
			size_t totalPointsSize = nPoints * sizeof( ExtendedPoint );
			size_t totalNodesSize = nNodes * sizeof( MediumInnerNode< ExtendedPointVector > );
			size_t maxMemToUse = totalMortonsSize + totalPointsSize + totalNodesSize;
			
			BitMapMemoryManager::initInstance( maxMemToUse );
			BitMapMemoryManager& manager = dynamic_cast< BitMapMemoryManager& >( BitMapMemoryManager::instance() );
			
			ASSERT_EQ( 0, manager.usedMemory() );
			
			MediumOctreeMap map;
			
			for( unsigned int i = 0u; i < nNodes; ++i )
			{
				MediumMortonCodePtr mortonCode( new MediumMortonCode() );
				mortonCode->build( i );
				
				MediumInnerNodePtr< ExtendedPointVector > node( new MediumInnerNode< ExtendedPointVector >() );
				ExtendedPointPtr p0( new ExtendedPoint() );
				ExtendedPointPtr p1( new ExtendedPoint() );
				node->setContents( ExtendedPointVector( { p0, p1 } ) );
				
				map[ mortonCode ] = node;
			}
			
			ASSERT_EQ( maxMemToUse, manager.usedMemory() );
			
			map.clear();
			
			ASSERT_EQ( 0, manager.usedMemory() );
		}
	}
}