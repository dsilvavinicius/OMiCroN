#include <gtest/gtest.h>
#include <iostream>

#include "O1OctreeNode.h"

using namespace std;

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
		
		void initPoints( Array< ExtendedPointPtr >& points, int offset )
		{
			for( int i = 0; i < points.size(); i++ )
			{
				float value = offset + i;
				Vec3 vec( value, value, value );
				
				ExtendedPoint point( vec, vec, vec );
				points[ i ] = makeManaged< ExtendedPoint >( point );
			}
		}
		
		typedef Types< Point, ExtendedPoint > PointTypes;
		
		TYPED_TEST_CASE( O1OctreeNodeTest, PointTypes );
		
		TYPED_TEST( O1OctreeNodeTest, CreationAndLinks )
		{
			using PointPtr = shared_ptr< TypeParam >;
			using PointArray = Array< PointPtr >;
			using OctreeNode = O1OctreeNode< PointPtr >;
			using ChildArray = Array< OctreeNode >;
			
			ASSERT_EQ( AllocStatistics::totalAllocated(), 0 );
			
			{
				int nPoints = 10;
				PointArray points( nPoints ); initPoints( points, 0 );
				
				int nSiblings = 3;
				ChildArray siblings( nSiblings );
				for( int i = 0; i < nSiblings; ++i )
				{
					OctreeNode sibling( points, false );
					siblings[ i ] = sibling;
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
				
				node->setChildren( children );
				
				ASSERT_EQ( node->parent(), parent );
				ASSERT_EQ( node->leftSibling(), siblings.data() );
				ASSERT_EQ( node->rightSibling(), node + 1 );
			
				ASSERT_EQ( node->getContents(), points );
				ASSERT_EQ( node->parent()->getContents(), parentPoints );
				ASSERT_EQ( node->leftSibling()->getContents(), points );
				ASSERT_EQ( node->rightSibling()->getContents(), points );
				ASSERT_EQ( node->child()[ 0 ].getContents(), childPointArrays[ 0 ] );
				ASSERT_EQ( node->child()[ 0 ].rightSibling()->getContents(), childPointArrays[ 1 ] );
				ASSERT_EQ( node->child()[ 0 ].rightSibling()->rightSibling()->getContents(), childPointArrays[ 2 ] );
				
				delete parent;
			}
			
			ASSERT_EQ( AllocStatistics::totalAllocated(), 0 );
		}
		
		/*TYPED_TEST( O1OctreeNodeTest, Serialization )
		{
			
		}*/
	}
}