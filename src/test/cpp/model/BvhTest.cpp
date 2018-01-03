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
		
		/** Tests the entire LeafAabb's API. */
		TEST_F( BvhTest, LeafAabb )
		{
			Point p0( Vec3( 1.f, 1.f, 1.f ), Vec3( 1.f, 1.f, 1.f ) );
			
			LeafAabb aabb( p0 );
			
			ASSERT_TRUE( aabb.isLeaf() );
			
			Point p1( Vec3( 2.f, 2.f, 2.f ), Vec3( 2.f, -1.f, 3.f ) );
			aabb.insert( p1 );
			
			Point p2( Vec3( 3.f, 3.f, 3.f ), Vec3( 1.5, 0.f, 2.f ) );
			aabb.traverseAndInsert( p2 );
			
			const Aabb::Boundaries& boundaries = aabb.boundaries();
			
			cout << "Boundaries: " << endl << boundaries << endl << endl;
			
			ASSERT_TRUE( aabb.origin().isApprox( Vec3( 1.f, -1.f, 1.f ) ) );
			ASSERT_TRUE( boundaries.m_origin.isApprox( Vec3( 1.f, -1.f, 1.f ) ) );
			ASSERT_TRUE( boundaries.m_extension.isApprox( Vec3( 1.f, 2.f, 2.f ) ) );
			
			Vec3 center = aabb.center();
			
			ASSERT_TRUE( center.isApprox( Vec3( 1.5f, 0.f, 2.f ) ) );
			
			const vector< Point >& points = aabb.points();
			ASSERT_EQ( points.size(), 3 );
			ASSERT_TRUE( points[ 0 ].equal( p0 ) );
			ASSERT_TRUE( points[ 1 ].equal( p1 ) );
			ASSERT_TRUE( points[ 2 ].equal( p2 ) );
		}
		
		TEST_F( BvhTest, InnerAabbSurfaceArea )
		{
			Point p0( Vec3( 0.f, 0.f, 0.f ), Vec3( -1.f, -1.f, -1.f ) );
			Point p1( Vec3( 1.f, 1.f, 1.f ), Vec3( 1.f, 1.f, 1.f ) );
			
			Aabb::AabbPtr child0( new LeafAabb( p0 ) );
			Aabb::AabbPtr child1( new LeafAabb( p1 ) );
			
			InnerAabb parent( child0, child1 );

			ASSERT_FALSE( parent.isLeaf() );
			
			float parentSurfaceArea = parent.sahSurfaceArea( parent.boundaries() );
			float child0SurfaceArea = child0->sahSurfaceArea( child0->boundaries() );
			float child1SurfaceArea = child1->sahSurfaceArea( child1->boundaries() );
			
			cout << "Parent SA: " << parentSurfaceArea << " Child0 SA: " << child0SurfaceArea << " Child1 SA: " << child1SurfaceArea << endl << endl;
			
			ASSERT_LT( child0SurfaceArea, parentSurfaceArea );
			ASSERT_LT( child1SurfaceArea, parentSurfaceArea );
			
			ASSERT_TRUE( parent.origin().isApprox( Vec3( -1.f, -1.f, -1.f ) ) );
			ASSERT_TRUE( parent.extension().isApprox( Vec3( 2.f, 2.f, 2.f ) ) );
		}
		
		TEST_F( BvhTest, BvhInsertion0 )
		{
			Point p0( Vec3( 0.f, 0.f, 0.f ), Vec3( -1.f, -1.f, -1.f ) );
			Point p1( Vec3( 1.f, 1.f, 1.f ), Vec3( 1.f, 1.f, 1.f ) );
			Point p2( Vec3( 2.f, 2.f, 2.f ), Vec3( 2.f, 2.f, 2.f ) );
			
			Bvh bvh;
			bvh.insert( p0 );
			bvh.insert( p1 );
			bvh.insert( p2 );
			
			const Aabb& root = bvh.root();
			
			ASSERT_TRUE( root.origin().isApprox( Vec3( -1.f, -1.f, -1.f ) ) );
			ASSERT_TRUE( root.extension().isApprox( Vec3( 3.f, 3.f, 3.f ) ) );
			ASSERT_FALSE( root.isLeaf() );
			
			const InnerAabb::ChildrenVector& rootsChildren = dynamic_cast< const InnerAabb& >( root ).children();
			ASSERT_EQ( rootsChildren.size(), 2 );
			
			const Aabb& expectedP0 = *rootsChildren[ 0 ];
			ASSERT_TRUE( expectedP0.isLeaf() );
			ASSERT_TRUE( expectedP0.origin().isApprox( p0.getPos() ) );
			ASSERT_TRUE( expectedP0.extension().isApprox( Vec3( 0.f, 0.f, 0.f ) ) );
			
			const Aabb& expectedP1P2Parent = *rootsChildren[ 1 ];
			ASSERT_FALSE( expectedP1P2Parent.isLeaf() );
			ASSERT_TRUE( expectedP1P2Parent.origin().isApprox( p1.getPos() ) );
			ASSERT_TRUE( expectedP1P2Parent.extension().isApprox( Vec3( 1.f, 1.f, 1.f ) ) );
			
			const InnerAabb::ChildrenVector& p1P2ParentsChildren = dynamic_cast< const InnerAabb& >( expectedP1P2Parent ).children();
			ASSERT_EQ( p1P2ParentsChildren.size(), 2 );
			
			const Aabb& expectedP1 = *p1P2ParentsChildren[ 0 ];
			ASSERT_TRUE( expectedP1.isLeaf() );
			ASSERT_TRUE( expectedP1.origin().isApprox( p1.getPos() ) );
			ASSERT_TRUE( expectedP1.extension().isApprox( Vec3( 0.f, 0.f, 0.f ) ) );
			
			const Aabb& expectedP2 = *p1P2ParentsChildren[ 1 ];
			ASSERT_TRUE( expectedP2.isLeaf() );
			ASSERT_TRUE( expectedP2.origin().isApprox( p2.getPos() ) );
			ASSERT_TRUE( expectedP2.extension().isApprox( Vec3( 0.f, 0.f, 0.f ) ) );
			
			ASSERT_NO_THROW( bvh.isSane() );
		}
		
		TEST_F( BvhTest, BvhInsertion1 )
		{
			Point p0( Vec3( 0.f, 0.f, 0.f ), Vec3( -1.f, -1.f, -1.f ) );
			Point p1( Vec3( 1.f, 1.f, 1.f ), Vec3( 1.f, 1.f, 1.f ) );
			Point p2( Vec3( 2.f, 2.f, 2.f ), Vec3( 2.f, 2.f, 0.5f ) );
			
			Bvh bvh;
			bvh.insert( p0 );
			bvh.insert( p1 );
			bvh.insert( p2 );
			
			const Aabb& root = bvh.root();
			
			ASSERT_TRUE( root.origin().isApprox( Vec3( -1.f, -1.f, -1.f ) ) );
			ASSERT_TRUE( root.extension().isApprox( Vec3( 3.f, 3.f, 2.f ) ) );
			ASSERT_FALSE( root.isLeaf() );
			
			const InnerAabb::ChildrenVector& rootsChildren = dynamic_cast< const InnerAabb& >( root ).children();
			ASSERT_EQ( rootsChildren.size(), 2 );
			
			const Aabb& expectedP0 = *rootsChildren[ 0 ];
			ASSERT_TRUE( expectedP0.isLeaf() );
			ASSERT_TRUE( expectedP0.origin().isApprox( p0.getPos() ) );
			ASSERT_TRUE( expectedP0.extension().isApprox( Vec3( 0.f, 0.f, 0.f ) ) );
			
			const Aabb& expectedP1P2Parent = *rootsChildren[ 1 ];
			ASSERT_FALSE( expectedP1P2Parent.isLeaf() );
			ASSERT_TRUE( expectedP1P2Parent.origin().isApprox( Vec3( 1.f, 1.f, 0.5f ) ) );
			ASSERT_TRUE( expectedP1P2Parent.extension().isApprox( Vec3( 1.f, 1.f, 0.5f ) ) );
			
			const InnerAabb::ChildrenVector& p1P2ParentsChildren = dynamic_cast< const InnerAabb& >( expectedP1P2Parent ).children();
			ASSERT_EQ( p1P2ParentsChildren.size(), 2 );
			
			const Aabb& expectedP1 = *p1P2ParentsChildren[ 0 ];
			ASSERT_TRUE( expectedP1.isLeaf() );
			ASSERT_TRUE( expectedP1.origin().isApprox( p1.getPos() ) );
			ASSERT_TRUE( expectedP1.extension().isApprox( Vec3( 0.f, 0.f, 0.f ) ) );
			
			const Aabb& expectedP2 = *p1P2ParentsChildren[ 1 ];
			ASSERT_TRUE( expectedP2.isLeaf() );
			ASSERT_TRUE( expectedP2.origin().isApprox( p2.getPos() ) );
			ASSERT_TRUE( expectedP2.extension().isApprox( Vec3( 0.f, 0.f, 0.f ) ) );
			
			ASSERT_NO_THROW( bvh.isSane() );
		}
		
		TEST_F( BvhTest, BvhInsertion2 )
		{
			Point p0( Vec3( 0.f, 0.f, 0.f ), Vec3( -1.f, -1.f, 0.f ) );
			Point p1( Vec3( 2.f, 2.f, 2.f ), Vec3( 2.f, 2.f, -1.0f ) );
			Point p2( Vec3( 1.f, 1.f, 1.f ), Vec3( 0.f, 0.f, 1.f ) );
			
			Bvh bvh;
			bvh.insert( p0 );
			bvh.insert( p1 );
			bvh.insert( p2 );
			
			const Aabb& root = bvh.root();
			
			ASSERT_TRUE( root.origin().isApprox( Vec3( -1.f, -1.f, -1.f ) ) );
			ASSERT_TRUE( root.extension().isApprox( Vec3( 3.f, 3.f, 2.f ) ) );
			ASSERT_FALSE( root.isLeaf() );
			
			const InnerAabb::ChildrenVector& rootsChildren = dynamic_cast< const InnerAabb& >( root ).children();
			ASSERT_EQ( rootsChildren.size(), 2 );
			
			const Aabb& expectedP0P2Parent = *rootsChildren[ 0 ];
			ASSERT_FALSE( expectedP0P2Parent.isLeaf() );
			ASSERT_TRUE( expectedP0P2Parent.origin().isApprox( Vec3( -1.f, -1.f, 0.f ) ) );
			ASSERT_TRUE( expectedP0P2Parent.extension().isApprox( Vec3( 1.f, 1.f, 1.f ) ) );
			
			const Aabb& expectedP1 = *rootsChildren[ 1 ];
			ASSERT_TRUE( expectedP1.isLeaf() );
			ASSERT_TRUE( expectedP1.origin().isApprox( p1.getPos() ) );
			ASSERT_TRUE( expectedP1.extension().isApprox( Vec3( 0.f, 0.f, 0.f ) ) );
			
			const InnerAabb::ChildrenVector& p0P2ParentsChildren = dynamic_cast< const InnerAabb& >( expectedP0P2Parent ).children();
			ASSERT_EQ( p0P2ParentsChildren.size(), 2 );
			
			const Aabb& expectedP0 = *p0P2ParentsChildren[ 0 ];
			ASSERT_TRUE( expectedP0.isLeaf() );
			ASSERT_TRUE( expectedP0.origin().isApprox( p0.getPos() ) );
			ASSERT_TRUE( expectedP0.extension().isApprox( Vec3( 0.f, 0.f, 0.f ) ) );
			
			const Aabb& expectedP2 = *p0P2ParentsChildren[ 1 ];
			ASSERT_TRUE( expectedP2.isLeaf() );
			ASSERT_TRUE( expectedP2.origin().isApprox( p2.getPos() ) );
			ASSERT_TRUE( expectedP2.extension().isApprox( Vec3( 0.f, 0.f, 0.f ) ) );
			
			ASSERT_NO_THROW( bvh.isSane() );
		}
		
// 		TEST_F( BvhTest, BvhInsertion3 )
// 		{
// 			Point p0( Vec3( 0.f, 0.f, 0.f ), Vec3( -1.f, -1.f, -1.f ) );
// 			Point p1( Vec3( 2.f, 2.f, 2.f ), Vec3( 2.f, 2.f, 1.f ) );
// 			Point p2( Vec3( 1.f, 1.f, 1.f ), Vec3( 0.f, 0.f, 0.5f ) );
// 			
// 			Bvh bvh;
// 			bvh.insert( p0 );
// 			bvh.insert( p1 );
// 			bvh.insert( p2 );
// 			
// 			const Aabb& root = bvh.root();
// 			
// 			ASSERT_TRUE( root.origin().isApprox( Vec3( -1.f, -1.f, -1.f ) ) );
// 			ASSERT_TRUE( root.extension().isApprox( Vec3( 3.f, 3.f, 2.f ) ) );
// 			ASSERT_FALSE( root.isLeaf() );
// 			
// 			const InnerAabb::ChildrenVector& rootsChildren = dynamic_cast< const InnerAabb& >( root ).children();
// 			ASSERT_EQ( rootsChildren.size(), 3 );
// 			
// 			const Aabb& expectedP0 = *rootsChildren[ 0 ];
// 			ASSERT_TRUE( expectedP0.isLeaf() );
// 			ASSERT_TRUE( expectedP0.origin().isApprox( p0.getPos() ) );
// 			ASSERT_TRUE( expectedP0.extension().isApprox( Vec3( 0.f, 0.f, 0.f ) ) );
// 			
// 			const Aabb& expectedP1 = *rootsChildren[ 1 ];
// 			ASSERT_TRUE( expectedP1.isLeaf() );
// 			ASSERT_TRUE( expectedP1.origin().isApprox( p1.getPos() ) );
// 			ASSERT_TRUE( expectedP1.extension().isApprox( Vec3( 0.f, 0.f, 0.f ) ) );
// 			
// 			const Aabb& expectedP2 = *rootsChildren[ 2 ];
// 			ASSERT_TRUE( expectedP2.isLeaf() );
// 			ASSERT_TRUE( expectedP2.origin().isApprox( p2.getPos() ) );
// 			ASSERT_TRUE( expectedP2.extension().isApprox( Vec3( 0.f, 0.f, 0.f ) ) );
// 			
// 			testAabb( root );
// 		}
		
		TEST_F( BvhTest, BvhReal )
		{
// 			Bvh bvh( "../data/example/staypuff.ply" );
// 			Bvh bvh( "/home/vinicius/Projects/PointBasedGraphics/Cumulus/src/data/real/prova5M.ply" );
			Bvh bvh( "/media/vinicius/Expansion Drive3/Datasets/bunny/bunny/reconstruction/bun_zipper_normals_bin.ply" );
			
			const Aabb& root = bvh.root();
			
			bvh.isSane();
			
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