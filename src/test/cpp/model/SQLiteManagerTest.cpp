#include <gtest/gtest.h>
#include "SQLiteManager.h"
#include "MortonCode.h"

namespace model
{
	namespace test
	{
        class SQLiteManagerTest
        : public ::testing::Test
		{
		protected:
			void SetUp(){}
		};
		
		TEST_F( SQLiteManagerTest, InsertAndGetPoints )
		{
			using OctreeNode = model::ShallowOctreeNode;
			using SQLiteManager = util::SQLiteManager< Point, ShallowMortonCode, OctreeNode >;
			
			Point p0( vec3( 11.321565f, 4.658535f, 7.163479f ), vec3( 7.163479f, 4.658535f, 11.321565f ) );
			Point p1( vec3( 11.201763f, 5.635769f, 6.996898f ), vec3( 6.996898f, 5.635769f, 11.201763f ) );
			Point p2( vec3( 11.198129f, 4.750132f, 7.202037f ), vec3( 7.202037f, 4.750132f, 11.198129f ) );
			
			SQLiteManager sqLite;
			sqLite.insertPoint( p0 );
			sqLite.insertPoint( p1 );
			sqLite.insertPoint( p2 );
			
			float epsilon = 1.e-15;
			Point* p = sqLite.getPoint( 0 );
			ASSERT_TRUE( p->equal( p0, epsilon ) );
			delete p;
			
			p = sqLite.getPoint( 1 );
			ASSERT_TRUE( p->equal( p1, epsilon ) );
			delete p;
			
			p = sqLite.getPoint( 2 ); 
			ASSERT_TRUE( p->equal( p2, epsilon ) );
			delete p;
		}
		
		TEST_F( SQLiteManagerTest, InsertAndGetIndexNodes )
		{
			using MortonCode = model::ShallowMortonCode;
			using Contents = vector< int >;
			using OctreeNode = model::ShallowOctreeNode;
			using LeafNode = model::ShallowLeafNode< Contents >;
			using InnerNode = model::ShallowInnerNode< Contents >;
			using SQLiteManager = util::SQLiteManager< Point, MortonCode, OctreeNode >;
			
			MortonCode leafCode;
			leafCode.build( 2, 3, 4, 5 );
			LeafNode leafNode;
			Contents leafPoints = { 5, 4, 3, 2 };
			leafNode.setContents( leafPoints );
			
			SQLiteManager sqLite;
			sqLite.insertNode< Contents >( leafCode, leafNode );
			
			MortonCode innerCode;
			innerCode.build( 1, 2, 3, 4 );
			InnerNode innerNode;
			Contents innerPoints = { 4, 3, 2, 1 };
			innerNode.setContents( innerPoints );
			
			sqLite.insertNode< Contents >( innerCode, innerNode );
			
			OctreeNode* queriedNode = sqLite.getNode< Contents >( leafCode );
			ASSERT_EQ( *queriedNode->getContents< Contents >(), leafPoints );
			delete queriedNode;
			
			queriedNode = sqLite.getNode< Contents >( innerCode );
			ASSERT_EQ( *queriedNode->getContents< Contents >(), innerPoints );
			delete queriedNode;
		}
		
		TEST_F( SQLiteManagerTest, InsertAndGetPointNodes )
		{
			using OctreeNode = model::ShallowOctreeNode;
			using LeafNode = model::LeafNode< ShallowMortonCode, PointVector >;
			using SQLiteManager = util::SQLiteManager< Point, ShallowMortonCode, OctreeNode >;
			
			Point p0( vec3( 11.321565f, 4.658535f, 7.163479f ), vec3( 7.163479f, 4.658535f, 11.321565f ) );
			Point p1( vec3( 11.201763f, 5.635769f, 6.996898f ), vec3( 6.996898f, 5.635769f, 11.201763f ) );
			Point p2( vec3( 11.198129f, 4.750132f, 7.202037f ), vec3( 7.202037f, 4.750132f, 11.198129f ) );
			
			PointVector points;
			points.push_back( make_shared< Point >( p0 ) );
			points.push_back( make_shared< Point >( p1 ) );
			points.push_back( make_shared< Point >( p2 ) );
			
			LeafNode node;
			node.setContents( points );
			
			ShallowMortonCode code;
			code.build( 1, 2, 3, 4 );
			SQLiteManager sqLite;
			
			sqLite.insertNode< PointVector >( code, node );
			
			OctreeNode* queriedNode = sqLite.getNode< PointVector >( code );
			
			float epsilon = 1.e-15;
			PointVector queriedPoints = *queriedNode->getContents< PointVector >();
			
			for( int i = 0; i < points.size(); ++i )
			{
				ASSERT_TRUE( points[ i ]->equal( *queriedPoints[ i ], epsilon ) );
			}
			
			delete queriedNode;
		}
		
		TEST_F( SQLiteManagerTest, InsertAndGetIdNodes )
		{
			using Contents = vector< int >;
			using OctreeNode = model::ShallowOctreeNode;
			using LeafNode = model::LeafNode< ShallowMortonCode, Contents >;
			using SQLiteManager = util::SQLiteManager< Point, ShallowMortonCode, OctreeNode >;
			using IdNode = util::IdNode< ShallowMortonCode >;
			
			int rawInts0[ 3 ] = { 1, 2, 3 };
			int rawInts1[ 3 ] = { 10, 20, 30 };
			Contents ints0( rawInts0, rawInts0 + 3 );
			Contents ints1( rawInts1, rawInts1 + 3 );
			
			LeafNode node0;
			LeafNode node1;
			node0.setContents( ints0 );
			node1.setContents( ints1 );
			
			ShallowMortonCode code0;
			code0.build( 1, 2, 3, 4 );
			ShallowMortonCode code1;
			code1.build( 7, 7, 7, 10 );
			ShallowMortonCode intervalEnd;
			intervalEnd.build( 1, 2, 3, 5 );
			
			SQLiteManager sqLite;
			sqLite.insertNode< Contents >( code0, node0 );
			sqLite.insertNode< Contents >( code1, node1 );
			
			vector< IdNode > queried = sqLite.getIdNodes< Contents >( code0, intervalEnd );
			
			ShallowMortonCode queriedId = *queried[ 0 ].first;
			ASSERT_EQ( queriedId, code0 );
			
			Contents queriedInts = *queried[ 0 ].second->getContents< Contents >();
			ASSERT_EQ( queriedInts, ints0 );
			
			delete queried[ 0 ].first;
			delete queried[ 0 ].second;
		}
		
		TEST_F( SQLiteManagerTest, DeleteNodes )
		{
			using MortonCode = model::ShallowMortonCode;
			using Contents = vector< int >;
			using OctreeNode = model::ShallowOctreeNode;
			using LeafNode = model::ShallowLeafNode< Contents >;
			using SQLiteManager = util::SQLiteManager< Point, MortonCode, OctreeNode >;
			
			MortonCode code0; code0.build( 0x8 );
			MortonCode code1; code1.build( 0x9 );
			MortonCode code2; code2.build( 0xA );
			MortonCode code3; code3.build( 0xB );
			
			LeafNode node0; node0.setContents( Contents( 3, 0 ) );
			LeafNode node1; node1.setContents( Contents( 3, 1 ) );
			LeafNode node2; node2.setContents( Contents( 3, 2 ) );
			LeafNode node3; node3.setContents( Contents( 3, 3 ) );
			
			SQLiteManager sqLite;
			sqLite.insertNode< Contents >( code0, node0 );
			sqLite.insertNode< Contents >( code1, node1 );
			sqLite.insertNode< Contents >( code2, node2 );
			sqLite.insertNode< Contents >( code3, node3 );
			
			sqLite.deleteNodes( code1, code2 );
			vector< OctreeNode* > queried = sqLite. template getNodes< Contents >( code0, code3 );
			
			ASSERT_EQ( queried.size(), 2 );
			ASSERT_EQ( *queried[ 0 ]->getContents< Contents >(), Contents( 3, 0 ) );
			delete queried[ 0 ];
			
			ASSERT_EQ( *queried[ 1 ]->getContents< Contents >(), Contents( 3, 3 ) );
			delete queried[ 1 ];
		}
		
		TEST_F( SQLiteManagerTest, InsertAndGetExtPointNodes )
		{
			using Point = ExtendedPoint;
			using PointVector = ExtendedPointVector;
			using OctreeNode = ShallowOctreeNode;
			using LeafNode = model::LeafNode< ShallowMortonCode, PointVector >;
			using SQLiteManager = util::SQLiteManager< Point, ShallowMortonCode, OctreeNode >;

			Point p0( vec3( 0.000510f, 0.000549f, 0.000588f ), vec3( 0.13f, 0.14f, 0.15f ), vec3( 9.f, 10.f, 24.f ) );
			Point p1( vec3( 0.04f, 0.05f, 0.06f ), vec3( 0.04f, 0.05f, 0.06f ), vec3( 3.f, -31.f ,4.f ) );
			Point p2( vec3( 0.07f, 0.08f, 0.09f ), vec3( 0.07f, 0.08f, 0.09f ), vec3( -14.f, 5.f ,6.f ) );
			
			PointVector points;
			points.push_back( make_shared< Point >( p0 ) );
			points.push_back( make_shared< Point >( p1 ) );
			points.push_back( make_shared< Point >( p2 ) );
			
			LeafNode node;
			node.setContents( points );
			
			ShallowMortonCode code;
			code.build( 1, 2, 3, 4 );
			SQLiteManager sqLite;
			
			sqLite.insertNode< PointVector >( code, node );
			
			OctreeNode* queriedNode = sqLite.getNode< PointVector >( code );
			
			float epsilon = 1.e-15;
			PointVector queriedPoints = *queriedNode->getContents< PointVector >();
			
			for( int i = 0; i < points.size(); ++i )
			{
				ASSERT_TRUE( points[ i ]->equal( *queriedPoints[ i ], epsilon ) );
			}
			
			delete queriedNode;
		}
		
		TEST_F( SQLiteManagerTest, InsertAndGetExtIdNodes )
		{
			using Contents = ExtendedPointVector;
			using LeafNode = model::LeafNode< ShallowMortonCode, Contents >;
			using SQLiteManager = util::SQLiteManager< ExtendedPoint, ShallowMortonCode, ShallowOctreeNode >;
			using IdNode = util::IdNode< ShallowMortonCode >;
			
			ExtendedPointPtr p0 = make_shared< ExtendedPoint >( vec3( 0.01f, 0.02f, 0.03f ), vec3( 0.01f, 0.02f, 0.03f ),
																vec3( 1.f, 15.f ,2.f ) );
			ExtendedPointPtr p1 = make_shared< ExtendedPoint >( vec3( 0.04f, 0.05f, 0.06f ), vec3( 0.04f, 0.05f, 0.06f ),
																vec3( 3.f, -31.f ,4.f ) );
			ExtendedPointPtr p2 = make_shared< ExtendedPoint >( vec3( 0.07f, 0.08f, 0.09f ), vec3( 0.07f, 0.08f, 0.09f ),
																vec3( -14.f, 5.f ,6.f ) );
			
			ExtendedPointPtr rawPoints0[ 3 ] = { p0, p1, p2 };
			ExtendedPointPtr rawPoints1[ 3 ] = { p2, p1, p0 };
			
			Contents vec0( rawPoints0, rawPoints0 + 3 );
			Contents vec1( rawPoints1, rawPoints1 + 3 );
			
			LeafNode node0;
			LeafNode node1;
			node0.setContents( vec0 );
			node1.setContents( vec1 );
			
			ShallowMortonCode code0;
			code0.build( 1, 2, 3, 4 );
			ShallowMortonCode code1;
			code1.build( 7, 7, 7, 10 );
			ShallowMortonCode intervalEnd;
			intervalEnd.build( 1, 2, 3, 5 );
			
			SQLiteManager sqLite;
			sqLite.insertNode< Contents >( code0, node0 );
			sqLite.insertNode< Contents >( code1, node1 );
			
			vector< IdNode > queried = sqLite.getIdNodes< Contents >( code0, intervalEnd );
			
			ShallowMortonCode queriedId = *queried[ 0 ].first;
			ASSERT_EQ( queriedId, code0 );
			
			float epsilon = 1.e-15;
			Contents queriedVec = *queried[ 0 ].second->getContents< Contents >();
			for( int i = 0; i < 3; ++i )
			{
				ASSERT_TRUE( vec0[ i ]->equal( *queriedVec[ i ], epsilon ) );
			}
			
			delete queried[ 0 ].first;
			delete queried[ 0 ].second;
		}
	}
}