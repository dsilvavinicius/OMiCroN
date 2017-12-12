#include "bvh/Bvh.h"

#include <gtest/gtest.h>
#include <iostream>

namespace model
{
	namespace test
	{
        class BvhTest : public ::testing::Test
		{
		protected:
			void SetUp() {}
		};

		TEST_F( BvhTest, Test )
		{
			Bvh bvh( "../data/example/staypuff.ply" );
			const Aabb& root = bvh.root();
			Bvh::Statistics stats = bvh.statistics();
			
			cout << "BVH Statistics: " << endl
				 << "Boundaries: " << endl << "origin: " << stats.m_boundaries.m_origin << endl << "extension" << stats.m_boundaries.m_extension << endl
				 << "Max depth: " << stats.m_maxDepth << endl
				 << "Number of nodes: " << stats.m_nNodes << endl
				 << "Number of points: " << stats.m_nPoints << endl
				 << "Recursion count: " << stats.m_recursionCount << endl << endl;
		}
	}
}