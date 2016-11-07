#include <gtest/gtest.h>
#include <iostream>

#include "O1OctreeNode.h"
#include "SQLiteManager.h"

using namespace std;
using namespace util;

namespace model
{
	namespace test
	{
		template< typename T >
        class O1OctreeNodeTest
        : public ::testing::Test
		{
		protected:
			void SetUp() {}
		};

		using testing::Types;
		
		void initPoints( Array< PointPtr >& points, int offset )
		{
			for( int i = 0; i < points.size(); i++ )
			{
				float value = offset + i;
				Vec3 vec( value, value, value );
				
				Point point( vec, vec );
				points[ i ] = makeManaged< Point >( point );
			}
		}
		
		typedef Types< Point > PointTypes;
		
		TYPED_TEST_CASE( O1OctreeNodeTest, PointTypes );
		
		TYPED_TEST( O1OctreeNodeTest, CreationAndLinks )
		{
			using PointPtr = shared_ptr< TypeParam >;
			using PointArray = Array< PointPtr >;
			using OctreeNode = O1OctreeNode< PointPtr >;
			using ChildArray = Array< OctreeNode >;
			
			ASSERT_EQ( AllocStatistics::totalAllocated(), 0 );
			
			{
				int nPoints = 1000;
				PointArray points( nPoints ); initPoints( points, 0 );
				
				int nSiblings = 3;
				ChildArray siblings( nSiblings );
				for( int i = 0; i < nSiblings; ++i )
				{
					siblings[ i ] = OctreeNode( points, false );
				}
				
				OctreeNode* node = siblings.data() + 1;
				
				PointArray parentPoints( nPoints ); initPoints( parentPoints, nPoints );
				
				auto parent = new OctreeNode( parentPoints, false );
				node->setParent( parent );
				
				int nChildren = 3;
				ChildArray children( nChildren );
				Array< PointArray > childPointArrays( nChildren );
				for( int i = 0; i < nChildren; ++i )
				{
					PointArray childPoints( nPoints ); initPoints( childPoints, nPoints * ( i + 1 ) );
					childPointArrays[ i ] = childPoints;
					children[ i ] = OctreeNode( std::move( childPoints ), true );
				}
				
				node->setChildren( std::move( children ) );
				
				ASSERT_EQ( node->getContents(), points );
				
				PointArray points2( points );
				node->setContents( std::move( points2 ) );
				
				ASSERT_EQ( node->getContents(), points );
				
				ASSERT_EQ( node->parent(), parent );
				ASSERT_EQ( node->leftSibling(), siblings.data() );
				ASSERT_EQ( node->rightSibling(), node + 1 );
			
				ASSERT_EQ( node->parent()->getContents(), parentPoints );
				ASSERT_EQ( node->leftSibling()->getContents(), points );
				ASSERT_EQ( node->rightSibling()->getContents(), points );
				ASSERT_EQ( node->child()[ 0 ].getContents(), childPointArrays[ 0 ] );
				ASSERT_EQ( node->child()[ 0 ].rightSibling()->getContents(), childPointArrays[ 1 ] );
				ASSERT_EQ( node->child()[ 0 ].rightSibling()->rightSibling()->getContents(), childPointArrays[ 2 ] );
				
				parent->turnLeaf();
				ASSERT_EQ( 0, parent->child().size() );
				ASSERT_EQ( true, parent->isLeaf() );
				
				delete parent;
			}
			
			ASSERT_EQ( AllocStatistics::totalAllocated(), 0 );
		}
		
		template< typename N >
		void assertNode( const N& expected, const N& other )
		{
			using PointArray = decltype( expected.getContents() );
			
			ASSERT_EQ( expected.isLeaf(), other.isLeaf() );
			
			PointArray expectedPoints = expected.getContents();
			PointArray otherPoints = other.getContents();
			
			ASSERT_EQ( expectedPoints.size(), otherPoints.size() );
			
			for( int i = 0; i < expectedPoints.size(); ++i )
			{
				ASSERT_TRUE( expectedPoints[ i ]->equal( *otherPoints[ i ], 1.e-15 ) );
			}
		}
		
		TYPED_TEST( O1OctreeNodeTest, Serialization )
		{
			using PointPtr = shared_ptr< TypeParam >;
			using PointArray = Array< PointPtr >;
			using Node = O1OctreeNode< PointPtr >;
			using Morton = ShallowMortonCode;
			using SqlManager = SQLiteManager< TypeParam, Morton, Node >;
			
			ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
			
			{
				int nPoints = 1000;
				
				PointArray points0( nPoints ); initPoints( points0, 0 );
				Node node0( points0, true );
				Morton code0; code0.build( 0x1000 );
				
				PointArray points1( nPoints ); initPoints( points1, nPoints );
				Node node1( points0, true );
				Morton code1; code1.build( 0x1001 );
				
				SqlManager sql( "Octree.db" );
				sql.insertNode( code0, node0 );
				sql.insertNode( code1, node1 );
				
				pair< bool, Node > queried = sql.getNode( code0 );
				ASSERT_TRUE( queried.first );
				
				assertNode( node0, queried.second );
				
				Array< Node > queriedNodes = sql.getNodes( code0, code1 );
				
				ASSERT_EQ( 2, queriedNodes.size() );
				assertNode( node0, queriedNodes[ 0 ] );
				assertNode( node1, queriedNodes[ 1 ] );
			}
			
			ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
		}
	}
}