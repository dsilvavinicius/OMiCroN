#include "Octree.h"

#include <gtest/gtest.h>

namespace model
{
	namespace test
	{
        class OctreeTest : public ::testing::Test
		{
		protected:
			/** Creates points that will be inside the octree. */
			void SetUp()
			{
				// These points should define the boundaries of the octree hexahedron.
				PointPtr< float, vec3 > up = make_shared< Point< float, vec3 > >(vec3(0.f, 0.f, 0.f), vec3(1.f, 15.f ,2.f));
				PointPtr< float, vec3 > down = make_shared< Point< float, vec3 > >(vec3(0.f, 0.f, 0.f), vec3(3.f, -31.f ,4.f));
				PointPtr< float, vec3 > left = make_shared< Point< float, vec3 > >(vec3(0.f, 0.f, 0.f), vec3(-14.f, 5.f ,6.f));
				PointPtr< float, vec3 > right = make_shared< Point< float, vec3 > >(vec3(0.f, 0.f, 0.f), vec3(46.f, 7.f ,8.f));
				PointPtr< float, vec3 > front = make_shared< Point< float, vec3 > >(vec3(0.f, 0.f, 0.f), vec3(9.f, 10.f ,24.f));
				PointPtr< float, vec3 > back = make_shared< Point< float, vec3 > >(vec3(0.f, 0.f, 0.f), vec3(11.f, 12.f ,-51.f));
				
				// Additional points inside the hexahedron.
				PointVector< float, vec3 > additionalPoints;
				additionalPoints.push_back(make_shared< Point< float, vec3 > >(vec3(0.f, 0.f, 0.f), vec3(13.f, -12.f, 9.f)));
				additionalPoints.push_back(make_shared< Point< float, vec3 > >(vec3(0.f, 0.f, 0.f), vec3(-5.f, -8.f, 1.f)));
				additionalPoints.push_back(make_shared< Point< float, vec3 > >(vec3(0.f, 0.f, 0.f), vec3(14.f, 11.f, -4.f)));
				additionalPoints.push_back(make_shared< Point< float, vec3 > >(vec3(0.f, 0.f, 0.f), vec3(7.f, 3.f, -12.f)));
				additionalPoints.push_back(make_shared< Point< float, vec3 > >(vec3(0.f, 0.f, 0.f), vec3(12.f, 5.f, 0.f)));
				
				m_points.push_back(back);
				m_points.push_back(front);
				m_points.push_back(right);
				m_points.push_back(left);
				m_points.push_back(down);
				m_points.push_back(up);
				m_points.insert(m_points.end(), additionalPoints.begin(), additionalPoints.end());
			}
			
			PointVector< float, vec3 > m_points;
		};

		/** Checks origin and size of built octree. */
		TEST_F(OctreeTest, Creation_Boundaries)
		{	
			ShallowOctreePtr<float, vec3> octree = make_shared< ShallowOctree<float, vec3> >(1);
			octree->build(m_points);
			
			ASSERT_EQ(octree->getMaxLevel(), 9);
			ASSERT_EQ(octree->getMaxPointsPerNode(), 1);
			
			vec3 origin = *octree->getOrigin();
			vec3 size = *octree->getSize();
			
			ASSERT_TRUE(glm::all(glm::equal(origin, vec3(-14.f, -31.f, -51.f))));
			ASSERT_TRUE(glm::all(glm::equal(size, vec3(60.f, 46.f, 75.f))));
		}
		
		TEST_F(OctreeTest, Creation_Hierarchy)
		{
			ShallowOctreePtr<float, vec3> octree = make_shared< ShallowOctree<float, vec3> >(1);
			octree->build(m_points);
			ShallowOctreeMapPtr< float, vec3 > hierarchy = octree->getHierarchy();
		}
	}
}