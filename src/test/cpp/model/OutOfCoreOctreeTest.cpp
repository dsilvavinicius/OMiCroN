#include <gtest/gtest.h>

#include "OutOfCoreOctree.h"

extern "C" string g_appPath;

namespace model
{
	namespace test
	{
        class OutOfCoreOctreeTest
        : public ::testing::Test
		{
		protected:
			void SetUp(){}
		};
		
		// Checks if octree creation is correctly using the MemoryManager.
		TEST_F( OutOfCoreOctreeTest, DISABLED_Creation )
		{
			MemoryManager& manager = MemoryManager::instance();
			
			// Save current manager setup.
			uint shallowBlocks = manager.numBlocks( MemoryManager::SHALLOW_MORTON );
			uint mediumBlocks = manager.numBlocks( MemoryManager::MEDIUM_MORTON );
			uint pointBlocks = manager.numBlocks( MemoryManager::POINT );
			uint extendedBlocks = manager.numBlocks( MemoryManager::EXTENDED_POINT );
			uint nodeBlocks = manager.numBlocks( MemoryManager::NODE );
			
			MemoryManager::initInstance( 20, 0, 30, 0, 20 );
			
			ShallowOutOfCoreOctree octree( 1, 10, g_appPath + "/Octree.db",
										   ShallowOutOfCoreOctree::MemorySetup( 0.1f, 0.2f, 1, 1, 1 ) );
			octree.buildFromFile( g_appPath + "/data/simple_point_octree.ply", SimplePointReader::SINGLE,
								  Attributes::COLORS );
			
			// Restore manager setup.
			MemoryManager::initInstance( shallowBlocks, mediumBlocks, pointBlocks, extendedBlocks, nodeBlocks );
		}
	}
}