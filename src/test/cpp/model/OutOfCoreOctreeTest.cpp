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
		
		// Checks if OutOfCoreOctree creation is correct.
		template< typename OutOfCoreOctree >
		void testCreation( OutOfCoreOctree& octree )
		{
			ShallowOctreeMapPtr hierarchy = octree.getHierarchy();
			SQLiteManager< Point, ShallowMortonCode, ShallowOctreeNode >& sqLite = octree.getSQLiteManager();
			
			//cout << "DB after octree creation: " << endl << sqLite.output< PointVector >() << endl;
			
			ShallowIdNodeVector nodes = sqLite.getIdNodes< PointVector >();
			
			for( ShallowIdNode node : nodes )
			{
				( *hierarchy )[ node.first ] = node.second;
			}
			
			checkHierarchy( hierarchy );
		}
		
		TEST_F( OutOfCoreOctreeTest, Creation0 )
		{
			MemoryManager::initInstance( 40, 0, 40, 0, 40 );
			
			ShallowOutOfCoreOctree octree( 1, 10, g_appPath + "/Octree.db",
										   ShallowOutOfCoreOctree::MemorySetup( 0.85111f, 0.8999f, 1, 1, 1 ) );
			octree.buildFromFile( g_appPath + "/data/simple_point_octree.ply", SimplePointReader::SINGLE,
								  Attributes::COLORS );
			
			testCreation( octree );
		}
		
		TEST_F( OutOfCoreOctreeTest, Creation2 )
		{
			MemoryManager::initInstance( 40, 0, 40, 0, 40 );
			
			ShallowOutOfCoreOctree octree( 1, 10, g_appPath + "/Octree.db",
										   ShallowOutOfCoreOctree::MemorySetup( 0.1f, 0.2f, 1, 1, 1 ) );
			octree.buildFromFile( g_appPath + "/data/simple_point_octree.ply", SimplePointReader::SINGLE,
								  Attributes::COLORS );
			
			testCreation( octree );
		}
		
		TEST_F( OutOfCoreOctreeTest, Creation3 )
		{
			MemoryManager::initInstance( 100, 0, 100, 0, 100 );
			
			ShallowOutOfCoreOctree octree( 1, 10, g_appPath + "/Octree.db",
										   ShallowOutOfCoreOctree::MemorySetup( 0.1f, 0.2f, 1, 1, 1 ) );
			octree.buildFromFile( g_appPath + "/data/simple_point_octree.ply", SimplePointReader::SINGLE,
								  Attributes::COLORS );
			
			testCreation( octree );
		}
		
		TEST_F( OutOfCoreOctreeTest, Creation4 )
		{
			MemoryManager::initInstance( 100, 0, 100, 0, 100 );
			
			ShallowOutOfCoreOctree octree( 1, 10, g_appPath + "/Octree.db",
										   ShallowOutOfCoreOctree::MemorySetup( 0.1f, 0.2f, 5, 5, 5 ) );
			octree.buildFromFile( g_appPath + "/data/simple_point_octree.ply", SimplePointReader::SINGLE,
								  Attributes::COLORS );
			
			testCreation( octree );
		}
	}
}