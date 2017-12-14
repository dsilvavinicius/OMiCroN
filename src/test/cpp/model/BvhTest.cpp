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

		void testAabb( const Aabb& aabb )
		{
			if( !aabb.isLeaf() )
			{
				using ChildrenVector = InnerAabb::ChildrenVector;
				
				const ChildrenVector& children = dynamic_cast< const InnerAabb* >( &aabb )->children();
				
				float sah = aabb.sahSurfaceArea( aabb.boundaries() );
				
				for( ChildrenVector::const_iterator it = children.begin(); it != children.end(); it++ )
				{
					const Aabb& child = ( **it );
					
					// Checking surface area.
					ASSERT_LT( child.sahSurfaceArea( child.boundaries() ), sah );
					
					// Checking inclusion.
					for( int i = 0; i < 3; ++i )
					{
						ASSERT_GE( child.origin()[ i ], aabb.origin()[ i ]  );
						ASSERT_LE( child.maxPoint()[ i ], aabb.maxPoint()[ i ]  );
					}
					
					for( ChildrenVector::const_iterator it2 = std::next( it, 1 ); it2 != children.end(); it2++ )
					{
						const Aabb& sibling = ( **it2 );
						
						// Checking no intersection between children.
						bool intersecting = true;
						
						for( int i = 0; i < 3; ++i )
						{
							if( child.origin()[ i ] < sibling.origin()[ i ]  )
							{
								if( child.maxPoint()[ i ] < sibling.origin()[ i ] )
								{
									intersecting = false;
									break;
								}
							}
							else
							{
								if( child.origin()[ i ] > sibling.maxPoint()[ i ] )
								{
									intersecting = false;
									break;
								}
							}
						}
						
						// Test
						if( intersecting )
						{
							ASSERT_FALSE( intersecting );
						}
					}
					
				}
			}
		}
		
		TEST_F( BvhTest, Test )
		{
// 			Bvh bvh( "../data/example/staypuff.ply" );
// 			Bvh bvh( "/home/vinicius/Projects/PointBasedGraphics/Cumulus/src/data/real/prova5M.ply" );
			Bvh bvh( "/media/vinicius/Expansion Drive3/Datasets/bunny/bunny/reconstruction/bun_zipper_normals_bin.ply" );
			
			const Aabb& root = bvh.root();
			
			testAabb( root );
			
			Bvh::Statistics stats = bvh.statistics();
			
			cout << "BVH Statistics: " << endl
				 << "Boundaries: " << endl << "origin: " << endl << stats.m_boundaries.m_origin << endl << "extension" << endl << stats.m_boundaries.m_extension << endl
				 << "Max depth: " << stats.m_maxDepth << endl
				 << "Number of nodes: " << stats.m_nNodes << endl
				 << "Number of points: " << stats.m_nPoints << endl
				 << "Number of leaves: " << stats.m_nLeaves << endl
				 << "Avg points per leaf: " << stats.m_avgPointsPerLeaf << endl
				 << "Min points in a leaf: " << stats.m_minPointsPerLeaf << endl
				 << "Max points in a leaf: " << stats.m_maxPointsPerLeaf << endl
				 << "Recursion count: " << stats.m_recursionCount << endl << endl;
		}
	}
}