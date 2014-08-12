#include "Octree.h"

#include <gtest/gtest.h>

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
			}
			
			PointVector< float, vec3 > m_points;
		};
		
		template<typename MortonPrecision, typename Float, typename Vec3>
		void checkNode(OctreeMapPtr< MortonPrecision, Float, Vec3 > hierarchy, const MortonPrecision& bits)
		{
			auto code = make_shared< MortonCode< MortonPrecision > >();
			code->build(bits);
			auto iter = hierarchy->find(code);
			ASSERT_FALSE(iter == hierarchy->end());
			hierarchy->erase(iter);
		}

		/** Checks octree generated boundaries and hierarchy. */
		TEST_F(OctreeTest, Creation)
		{	
			auto octree = make_shared< ShallowOctree<float, vec3> >(1);
			octree->build(m_points);
			
			ASSERT_EQ(octree->getMaxLevel(), 10);
			ASSERT_EQ(octree->getMaxPointsPerNode(), 1);
			
			vec3 origin = *octree->getOrigin();
			vec3 size = *octree->getSize();
			vec3 leafSize = *octree->getLeafSize();
			
			ASSERT_TRUE(glm::all(glm::equal(origin, vec3(-14.f, -31.f, -51.f))));
			ASSERT_TRUE(glm::all(glm::equal(size, vec3(60.f, 46.f, 75.f))));
			ASSERT_TRUE(glm::all(glm::equal(leafSize, vec3(0.05859375f, 0.044921875f, 0.073242188f))));
			
			/*
			Expected hierarchy. 0x1 is the root node. The blank spaces are merged nodes. A node with an arrow that
			points to nothing means that it is a sibling of the node at the same position at the line immediately
			above.
			
			0xa6c3 -> 	______ -> _____ -> ____ -> 0xa -> 0x1
			0xa6c0 ->
										   0x67 -> 0xc ->
			0xc325 -> 	______ -> _____ -> 0x61 ->
			0xc320 ->
						______ -> _____ -> 0x70 -> 0xe ->
						______ -> _____ -> 0x71 ->
						______ -> 0x39f -> 0x73 ->
						______ -> 0x39d -> 
						0x1d82 -> _____ -> 0x76 ->
						0x1d80 ->
			*/
			
			ShallowOctreeMapPtr< float, vec3 > hierarchy = octree->getHierarchy();
			
			SCOPED_TRACE("0xa6c3");
			checkNode< unsigned int, float, vec3 >(hierarchy, 0xa6c3u);
			
			SCOPED_TRACE("0xa6c0");
			checkNode< unsigned int, float, vec3 >(hierarchy, 0xa6c0u);
			
			SCOPED_TRACE("0xc325");
			checkNode< unsigned int, float, vec3 >(hierarchy, 0xc325u);
			
			SCOPED_TRACE("0xc320");
			checkNode< unsigned int, float, vec3 >(hierarchy, 0xc320u);
			
			SCOPED_TRACE("0x1d82");
			checkNode< unsigned int, float, vec3 >(hierarchy, 0x1d82u);
			
			SCOPED_TRACE("0x1d80");
			checkNode< unsigned int, float, vec3 >(hierarchy, 0x1d80u);
			
			SCOPED_TRACE("0x39f");
			checkNode< unsigned int, float, vec3 >(hierarchy, 0x39fu);
			
			SCOPED_TRACE("0x39d");
			checkNode< unsigned int, float, vec3 >(hierarchy, 0x39du);
			
			SCOPED_TRACE("0x67");
			checkNode< unsigned int, float, vec3 >(hierarchy, 0x67u);
			
			SCOPED_TRACE("0x61");
			checkNode< unsigned int, float, vec3 >(hierarchy, 0x61u);
			
			SCOPED_TRACE("0x70");
			checkNode< unsigned int, float, vec3 >(hierarchy, 0x70u);
			
			SCOPED_TRACE("0x71");
			checkNode< unsigned int, float, vec3 >(hierarchy, 0x71u);
			
			SCOPED_TRACE("0x73");
			checkNode< unsigned int, float, vec3 >(hierarchy, 0x73u);
			
			SCOPED_TRACE("0x76");
			checkNode< unsigned int, float, vec3 >(hierarchy, 0x76u);
			
			SCOPED_TRACE("0xa");
			checkNode< unsigned int, float, vec3 >(hierarchy, 0xau);
			
			SCOPED_TRACE("0xc");
			checkNode< unsigned int, float, vec3 >(hierarchy, 0xcu);
			
			SCOPED_TRACE("0xe");
			checkNode< unsigned int, float, vec3 >(hierarchy, 0xeu);
			
			SCOPED_TRACE("0x1");
			checkNode< unsigned int, float, vec3 >(hierarchy, 0x1u);
			
			ASSERT_TRUE(hierarchy->empty());
		}
	}
}