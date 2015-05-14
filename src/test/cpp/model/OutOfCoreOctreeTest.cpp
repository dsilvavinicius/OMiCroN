#include <gtest/gtest.h>

#include "OutOfCoreOctree.h"

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
		
		TEST_F( OutOfCoreOctreeTest, Hierarchy )
		{
			ShallowOutOfCoreOctree< float, vec3, Point< float, vec3 > > octree( 1, 10 );
		}
	}
}