#include <gtest/gtest.h>

#include "OutOfCoreOctree.h"
#include "HierarchyTestMethods.h"

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
			MemoryManager::initInstance( 20, 0, 20, 0, 20 );
			
			ShallowOutOfCoreOctree octree( 1, 10, g_appPath + "/Octree.db",
										   ShallowOutOfCoreOctree::MemorySetup( 0.71f, 0.79f, 1, 1, 1 ) );
			octree.buildFromFile( g_appPath + "/data/simple_point_octree.ply", SimplePointReader::SINGLE,
								  Attributes::COLORS );
			
			ShallowOctreeMapPtr hierarchy = octree.getHierarchy();
			SQLiteManager< Point, ShallowMortonCode, ShallowOctreeNode >& sqLite = octree.getSQLiteManager();
			
			ShallowMortonCode a; a.build( 0x1 );
			ShallowMortonCode b = ShallowMortonCode::getLvlLast( 10 );
			
			ShallowIdNodeVector nodes = sqLite.getIdNodes< PointVector >();
			
			cout << "Nodes in db: " << nodes.size() << endl;
			
			for( ShallowIdNode node : nodes )
			{
				cout << *node.first << endl;
				( *hierarchy )[ node.first ] = node.second;
			}
			
			checkHierarchy( hierarchy );
		}
	}
}