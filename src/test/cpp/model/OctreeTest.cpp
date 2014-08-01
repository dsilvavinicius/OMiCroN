#include "Octree.h"

#include <gtest/gtest.h>

namespace model
{
	namespace test
	{
        class OctreeTest : public ::testing::Test
		{
		protected:
			void SetUp() {}
		};

		TEST_F(OctreeTest, Creation) {
			PointPtr<vec3> point = make_shared< Point< vec3 > >(vec3(1.f, 1.f ,1.f), vec3(0.f, 0.f, 0.f));
			vector< PointPtr <vec3> > points(1);
			points[0] = point;
			
			ShallowOctree<float, vec3> octree;
			OctreeBase<unsigned int, float, vec3>* octreeBase = &octree;
			octreeBase->build(points);
		}
	}
}