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
			using Point = Point< float, vec3 >;
			
			ShallowOutOfCoreOctree< float, vec3, Point>  octree( 1, 10 );
			octree.buildFromFile( g_appPath + "/data/test_normals.ply", SimplePointReader::SINGLE, NORMALS );
			
			SQLiteManager< Point >& sqLite = octree.getSQLiteManager();
			Point p = sqLite.getPoint( 1 );
			cout << "Point at 0: " << p << endl;
		}
	}
}