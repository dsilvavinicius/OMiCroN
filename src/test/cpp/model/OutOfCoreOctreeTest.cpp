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
			using MortonCode = typename OutOfCoreOctree::MortonCode;
			using Point = typename OutOfCoreOctree::Point;
			using OctreeNode = typename OutOfCoreOctree::OctreeNode;
			using OctreeMapPtr = typename OutOfCoreOctree::OctreeMapPtr;
			using IdNode = model::IdNode< MortonCode, OctreeNode >;
			using IdNodeVector = model::IdNodeVector< MortonCode, OctreeNode >;
			
			OctreeMapPtr hierarchy = octree.getHierarchy();
			SQLiteManager< Point, MortonCode, OctreeNode >& sqLite = octree.getSQLiteManager();
			
			//cout << "DB after octree creation: " << endl << sqLite.output< PointVector >() << endl;
			
			IdNodeVector nodes = sqLite.getIdNodes();
			
			for( IdNode node : nodes )
			{
				( *hierarchy )[ node.first ] = node.second;
			}
			
			checkHierarchy( hierarchy );
		}
		
		TEST_F( OutOfCoreOctreeTest, Creation0 )
		{
			DefaultManager< ShallowMortonCode, Point, OctreeNode< PointVector > >
			::initInstance( 200 * sizeof( OctreeNode< PointVector > ) );
			
			SPOpS_OutOfCoreOctree octree( 1, 10, "Octree.db", SPOpS_OutOfCoreOctree::MemorySetup( 0.8f, 0.8999f, 1, 1, 1 ) );
			octree.buildFromFile( "data/simple_point_octree.ply", SimplePointReader::SINGLE );
			
			testCreation( octree );
		}
		
		TEST_F( OutOfCoreOctreeTest, Creation1 )
		{
			DefaultManager< ShallowMortonCode, Point, OctreeNode< PointVector > >
			::initInstance( 50 * sizeof( OctreeNode< PointVector > ) );
			
			SPOpS_OutOfCoreOctree octree( 1, 10, "Octree.db", SPOpS_OutOfCoreOctree::MemorySetup( 0.1f, 0.2f, 1, 1, 1 ) );
			octree.buildFromFile( "data/simple_point_octree.ply", SimplePointReader::SINGLE );
			
			testCreation( octree );
		}
		
		TEST_F( OutOfCoreOctreeTest, Creation2 )
		{
			DefaultManager< ShallowMortonCode, Point, OctreeNode< PointVector > >
			::initInstance( 100 * sizeof( OctreeNode< PointVector > ) );
			
			SPOpS_OutOfCoreOctree octree( 1, 10, "Octree.db", SPOpS_OutOfCoreOctree::MemorySetup( 0.1f, 0.2f, 1, 1, 1 ) );
			octree.buildFromFile( "data/simple_point_octree.ply", SimplePointReader::SINGLE );
			
			testCreation( octree );
		}
		
		TEST_F( OutOfCoreOctreeTest, Creation3 )
		{
			DefaultManager< ShallowMortonCode, Point, OctreeNode< PointVector > >
			::initInstance( 50 * sizeof( OctreeNode< PointVector > ) );
			
			SPOpS_OutOfCoreOctree octree( 1, 10, "Octree.db", SPOpS_OutOfCoreOctree::MemorySetup( 0.1f, 0.2f, 5, 5, 5 ) );
			octree.buildFromFile( "data/simple_point_octree.ply", SimplePointReader::SINGLE );
			
			testCreation( octree );
		}
	}
}