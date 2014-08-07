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

		/** Checks origin and size of built octree. */
		TEST_F(OctreeTest, Creation_Boundaries)
		{
			// These points should define the boundaries of the octree hexahedron.
			PointPtr< float, vec3 > up = make_shared< Point< float, vec3 > >(vec3(1.f, 15.f ,2.f), vec3(0.f, 0.f, 0.f));
			PointPtr< float, vec3 > down = make_shared< Point< float, vec3 > >(vec3(3.f, -31.f ,4.f), vec3(0.f, 0.f, 0.f));
			PointPtr< float, vec3 > left = make_shared< Point< float, vec3 > >(vec3(-14.f, 5.f ,6.f), vec3(0.f, 0.f, 0.f));
			PointPtr< float, vec3 > right = make_shared< Point< float, vec3 > >(vec3(46.f, 7.f ,8.f), vec3(0.f, 0.f, 0.f));
			PointPtr< float, vec3 > front = make_shared< Point< float, vec3 > >(vec3(9.f, 10.f ,24.f), vec3(0.f, 0.f, 0.f));
			PointPtr< float, vec3 > back = make_shared< Point< float, vec3 > >(vec3(11.f, 12.f ,-51.f), vec3(0.f, 0.f, 0.f));
			
			// Additional points inside the hexahedron.
			PointVector< float, vec3 > additionalPoints;
			additionalPoints.push_back(make_shared< Point< float, vec3 > >(vec3(13.f, -12.f, 9.f), vec3(0.f, 0.f, 0.f)));
			additionalPoints.push_back(make_shared< Point< float, vec3 > >(vec3(-5.f, -8.f, 1.f), vec3(0.f, 0.f, 0.f)));
			additionalPoints.push_back(make_shared< Point< float, vec3 > >(vec3(14.f, 11.f, -4.f), vec3(0.f, 0.f, 0.f)));
			additionalPoints.push_back(make_shared< Point< float, vec3 > >(vec3(7.f, 3.f, -12.f), vec3(0.f, 0.f, 0.f)));
			additionalPoints.push_back(make_shared< Point< float, vec3 > >(vec3(12.f, 5.f, 0.f), vec3(0.f, 0.f, 0.f)));
			
			PointVector< float, vec3 > points;
			points.push_back(back);
			points.push_back(front);
			points.push_back(right);
			points.push_back(left);
			points.push_back(down);
			points.push_back(up);
			
			shared_ptr< ShallowOctree<float, vec3> > octree = make_shared< ShallowOctree<float, vec3> >(1);
			octree->build(points);
			
			ASSERT_EQ(octree->getMaxLevel(), 9);
			ASSERT_EQ(octree->getMaxPointsPerNode(), 1);
			
			vec3 origin = *octree->getOrigin();
			vec3 size = *octree->getSize();
			
			ASSERT_TRUE(glm::all(glm::equal(origin, vec3(-14.f, -31.f, -51.f))));
			ASSERT_TRUE(glm::all(glm::equal(size, vec3(60.f, 46.f, 75.f))));
		}
		
		TEST_F(OctreeTest, Creation_Hierarchy)
		{
		}
	}
}