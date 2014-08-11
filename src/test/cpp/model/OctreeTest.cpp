#include "Octree.h"

#include <gtest/gtest.h>
#include <unordered_map>

namespace model
{
	namespace test
	{
        class OctreeTest : public ::testing::Test
		{
		protected:
			/** Creates points that will be inside the octree and the associated expected results of octree construction. */
			void SetUp()
			{
				// These points should define the boundaries of the octree hexahedron.
				auto up = make_shared< Point< float, vec3 > >(vec3(0.01f, 0.02f, 0.03f), vec3(1.f, 15.f ,2.f));
				auto down = make_shared< Point< float, vec3 > >(vec3(0.04f, 0.05f, 0.06f), vec3(3.f, -31.f ,4.f));
				auto left = make_shared< Point< float, vec3 > >(vec3(0.07f, 0.08f, 0.09f), vec3(-14.f, 5.f ,6.f));
				auto right = make_shared< Point< float, vec3 > >(vec3(0.1f, 0.11f, 0.12f), vec3(46.f, 7.f ,8.f));
				auto front = make_shared< Point< float, vec3 > >(vec3(0.13f, 0.14f, 0.15f), vec3(9.f, 10.f ,24.f));
				auto back = make_shared< Point< float, vec3 > >(vec3(0.16f, 0.17f, 0.18f), vec3(11.f, 12.f ,-51.f));
				
				// Additional points inside the hexahedron.
				auto addPoint0 = make_shared< Point< float, vec3 > >(vec3(0.19f, 0.2f, 0.21f), vec3(13.f, -12.f, 9.f));
				auto addPoint1 = make_shared< Point< float, vec3 > >(vec3(0.22f, 0.23f, 0.24f), vec3(-5.f, -8.f, 1.f));
				auto addPoint2 = make_shared< Point< float, vec3 > >(vec3(0.25f, 0.26f, 0.27f), vec3(14.f, 11.f, -4.f));
				auto addPoint3 = make_shared< Point< float, vec3 > >(vec3(0.28f, 0.29f, 0.30f), vec3(7.f, 3.f, -12.f));
				auto addPoint4 = make_shared< Point< float, vec3 > >(vec3(0.31f, 0.32f, 0.33f), vec3(12.f, 5.f, 0.f));
				
				m_points.push_back(back);
				m_points.push_back(front);
				m_points.push_back(right);
				m_points.push_back(left);
				m_points.push_back(down);
				m_points.push_back(up);
				
				m_points.push_back(addPoint0);
				m_points.push_back(addPoint1);
				m_points.push_back(addPoint2);
				m_points.push_back(addPoint3);
				m_points.push_back(addPoint4);
				
				// Expected leaf code table.
				// The numerical constants here are the final coordinates of the point in octree space.
				auto upCode 		= make_shared< ShallowMortonCode >(); upCode->build(2, 10, 7, 10);
				auto downCode 		= make_shared< ShallowMortonCode >(); downCode->build(2, 0, 7, 10);
				auto leftCode 		= make_shared< ShallowMortonCode >(); leftCode->build(0, 7, 7, 10);
				auto rightCode 		= make_shared< ShallowMortonCode >(); rightCode->build(10, 8, 7, 10);
				auto frontCode 		= make_shared< ShallowMortonCode >(); frontCode->build(3, 8, 10, 10);
				auto backCode 		= make_shared< ShallowMortonCode >(); backCode->build(4, 9, 0, 10);
				auto addPoint0Code 	= make_shared< ShallowMortonCode >(); addPoint0Code->build(4, 4, 8, 10);
				auto addPoint1Code 	= make_shared< ShallowMortonCode >(); addPoint1Code->build(1, 5, 6, 10);
				auto addPoint2Code 	= make_shared< ShallowMortonCode >(); addPoint2Code->build(4, 9, 7, 10);
				auto addPoint3Code 	= make_shared< ShallowMortonCode >(); addPoint3Code->build(3, 7, 5, 10);
				auto addPoint4Code 	= make_shared< ShallowMortonCode >(); addPoint4Code->build(4, 7, 6, 10);
				
				/*cout << "Expected full paths from each node to root. This will not be the final hierarchy "
						"since nodes can be merged at construction time. " << endl;
				upCode			->printPathToRoot(cout, true);
				downCode		->printPathToRoot(cout, true);
				leftCode		->printPathToRoot(cout, true);
				rightCode		->printPathToRoot(cout, true);
				frontCode		->printPathToRoot(cout, true);
				backCode		->printPathToRoot(cout, true);
				addPoint0Code	->printPathToRoot(cout, true);
				addPoint1Code	->printPathToRoot(cout, true);
				addPoint2Code	->printPathToRoot(cout, true);
				addPoint3Code	->printPathToRoot(cout, true);
				addPoint4Code	->printPathToRoot(cout, true);
				cout << endl;*/
			}
			
			PointVector< float, vec3 > m_points;
		};

		/** Checks origin and size of built octree. */
		TEST_F(OctreeTest, Creation_Boundaries)
		{	
			auto octree = make_shared< ShallowOctree<float, vec3> >(1);
			octree->build(m_points);
			
			cout << "Final octree: " << *octree;
			
			ASSERT_EQ(octree->getMaxLevel(), 10);
			ASSERT_EQ(octree->getMaxPointsPerNode(), 1);
			
			vec3 origin = *octree->getOrigin();
			vec3 size = *octree->getSize();
			vec3 leafSize = *octree->getLeafSize();
			
			ASSERT_TRUE(glm::all(glm::equal(origin, vec3(-14.f, -31.f, -51.f))));
			ASSERT_TRUE(glm::all(glm::equal(size, vec3(60.f, 46.f, 75.f))));
			ASSERT_TRUE(glm::all(glm::equal(leafSize, vec3(0.05859375f, 0.044921875f, 0.073242188f))));
		}
	}
}