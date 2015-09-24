#include <gtest/gtest.h>

#include "OutOfCoreOctree.h"
#include <Ken12MemoryManager.h>
#include "HierarchyTestMethods.h"

namespace model
{
	namespace test
	{
        class OutOfCoreOctreeTest
        : public ::testing::Test
		{};
		
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
		
		TEST_F( OutOfCoreOctreeTest, CreationKen12Manager0 )
		{
			Ken12MemoryManager< ShallowMortonCode, Point, ShallowInnerNode< PointVector >, ShallowLeafNode< PointVector > >
				::initInstance( 40, 40, 40, 40 );
			
			ShallowOutOfCoreOctree octree( 1, 10, "Octree.db", ShallowOutOfCoreOctree::MemorySetup( 0.85111f, 0.8999f, 1, 1, 1 ) );
			octree.buildFromFile( "data/simple_point_octree.ply", SimplePointReader::SINGLE,
								  Attributes::COLORS );
			
			testCreation( octree );
		}
		
		TEST_F( OutOfCoreOctreeTest, CreationKen12Manager2 )
		{
			Ken12MemoryManager< ShallowMortonCode, Point, ShallowInnerNode< PointVector >, ShallowLeafNode< PointVector > >
				::initInstance( 40, 40, 40, 40 );
			
			ShallowOutOfCoreOctree octree( 1, 10, "Octree.db", ShallowOutOfCoreOctree::MemorySetup( 0.1f, 0.2f, 1, 1, 1 ) );
			octree.buildFromFile( "data/simple_point_octree.ply", SimplePointReader::SINGLE,
								  Attributes::COLORS );
			
			testCreation( octree );
		}
		
		TEST_F( OutOfCoreOctreeTest, CreationKen12Manager3 )
		{
			Ken12MemoryManager< ShallowMortonCode, Point, ShallowInnerNode< PointVector >, ShallowLeafNode< PointVector > >
				::initInstance( 100, 100, 100, 100 );
			
			ShallowOutOfCoreOctree octree( 1, 10, "Octree.db", ShallowOutOfCoreOctree::MemorySetup( 0.1f, 0.2f, 1, 1, 1 ) );
			octree.buildFromFile( "data/simple_point_octree.ply", SimplePointReader::SINGLE,
								  Attributes::COLORS );
			
			testCreation( octree );
		}
		
		TEST_F( OutOfCoreOctreeTest, CreationKen12Manager4 )
		{
			Ken12MemoryManager< ShallowMortonCode, Point, ShallowInnerNode< PointVector >, ShallowLeafNode< PointVector > >
				::initInstance( 100, 100, 100, 100 );
			
			ShallowOutOfCoreOctree octree( 1, 10, "Octree.db", ShallowOutOfCoreOctree::MemorySetup( 0.1f, 0.2f, 5, 5, 5 ) );
			octree.buildFromFile( "data/simple_point_octree.ply", SimplePointReader::SINGLE,
								  Attributes::COLORS );
			
			testCreation( octree );
		}
	}
}