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
		
		TEST_F( OutOfCoreOctreeTest, Creation )
		{
			ShallowOutOfCoreOctree< float, vec3, Point< float, vec3 > > octree( 1, 10 );
			octree.buildFromFile( g_appPath + "/data/test_normals.ply", SimplePointReader::SINGLE, NORMALS );
		}
	}
}