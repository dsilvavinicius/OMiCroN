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
			void SetUp()
			{
				// Save MemoryManager setup.
				MemoryManager& manager = MemoryManager::instance();
				
				m_shallowBlocks = manager.numBlocks( MemoryManager::FOUR_BYTES );
				m_mediumBlocks = manager.numBlocks( MemoryManager::EIGHT_BYTES );
				m_pointBlocks = manager.numBlocks( MemoryManager::TWENTY_FOUR_BYTES );
				m_extendedBlocks = manager.numBlocks( MemoryManager::THIRTY_SIX );
				m_nodeBlocks = manager.numBlocks( MemoryManager::THIRTY_TWO );
			}
			
			void TearDown()
			{
				// Restore MemoryManager setup.
				MemoryManager::initInstance( m_shallowBlocks, m_mediumBlocks, m_pointBlocks, m_extendedBlocks,
											 m_nodeBlocks );
			}
			
			uint m_shallowBlocks;
			uint m_mediumBlocks;
			uint m_pointBlocks;
			uint m_extendedBlocks;
			uint m_nodeBlocks;
		};
		
		// Checks if octree creation is correctly using the MemoryManager.
		TEST_F( OutOfCoreOctreeTest, Creation )
		{
			MemoryManager::initInstance( 10, 0, 20, 0, 10 );
			
			ShallowOutOfCoreOctree octree( 1, 10, g_appPath + "/Octree.db",
										   ShallowOutOfCoreOctree::MemorySetup( 0.21f, 0.39f, 1, 1, 1 ) );
			octree.buildFromFile( g_appPath + "/data/simple_point_octree.ply", SimplePointReader::SINGLE,
								  Attributes::COLORS );
		}
	}
}