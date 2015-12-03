#include <gtest/gtest.h>
#include "FastParallelOctree.h"

namespace model
{
	namespace test
	{
        class FastParallelOctreeTest
        :  public ::testing::Test
        {};
		
		TEST_F( FastParallelOctreeTest, Creation )
		{
			using Octree = FastParallelOctree< ShallowMortonCode, Point >;
			
			Octree octree;
			octree.buildFromFile( "data/simple_point_octree.ply", 10 );
		}
	}
}