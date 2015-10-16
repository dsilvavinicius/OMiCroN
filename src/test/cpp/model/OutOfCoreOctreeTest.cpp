#include <gtest/gtest.h>

#include "OutOfCoreOctree.h"
#include <MemoryManagerTypes.h>
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
			SQLiteManager< Point, ShallowMortonCode, OctreeNode >& sqLite = octree.getSQLiteManager();
			
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
			BitMapMemoryManager< ShallowMortonCode, Point, InnerNode< PointVector >, LeafNode< PointVector > >
				::initInstance( 200 * sizeof( InnerNode< PointVector > ) );
			
			ShallowOutOfCoreOctree octree( 1, 10, "Octree.db", ShallowOutOfCoreOctree::MemorySetup( 0.8f, 0.8999f, 1, 1, 1 ) );
			octree.buildFromFile( "data/simple_point_octree.ply", SimplePointReader::SINGLE,
								  Attributes::COLORS );
			
			testCreation( octree );
		}
		
		TEST_F( OutOfCoreOctreeTest, Creation1 )
		{
			BitMapMemoryManager< ShallowMortonCode, Point, InnerNode< PointVector >, LeafNode< PointVector > >
				::initInstance( 50 * sizeof( InnerNode< PointVector > ) );
			
			ShallowOutOfCoreOctree octree( 1, 10, "Octree.db", ShallowOutOfCoreOctree::MemorySetup( 0.1f, 0.2f, 1, 1, 1 ) );
			octree.buildFromFile( "data/simple_point_octree.ply", SimplePointReader::SINGLE,
								  Attributes::COLORS );
			
			testCreation( octree );
		}
		
		TEST_F( OutOfCoreOctreeTest, Creation2 )
		{
			BitMapMemoryManager< ShallowMortonCode, Point, InnerNode< PointVector >, LeafNode< PointVector > >
				::initInstance( 100 * sizeof( InnerNode< PointVector > ) );
			
			ShallowOutOfCoreOctree octree( 1, 10, "Octree.db", ShallowOutOfCoreOctree::MemorySetup( 0.1f, 0.2f, 1, 1, 1 ) );
			octree.buildFromFile( "data/simple_point_octree.ply", SimplePointReader::SINGLE,
								  Attributes::COLORS );
			
			testCreation( octree );
		}
		
		TEST_F( OutOfCoreOctreeTest, Creation3 )
		{
			BitMapMemoryManager< ShallowMortonCode, Point, InnerNode< PointVector >, LeafNode< PointVector > >
				::initInstance( 50 * sizeof( InnerNode< PointVector > ) );
			
			ShallowOutOfCoreOctree octree( 1, 10, "Octree.db", ShallowOutOfCoreOctree::MemorySetup( 0.1f, 0.2f, 5, 5, 5 ) );
			octree.buildFromFile( "data/simple_point_octree.ply", SimplePointReader::SINGLE,
								  Attributes::COLORS );
			
			testCreation( octree );
		}
	}
}