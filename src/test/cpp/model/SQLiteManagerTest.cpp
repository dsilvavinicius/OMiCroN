#include <gtest/gtest.h>
#include <random>
#include "SQLiteManager.h"
#include "MortonCode.h"

extern "C" string g_appPath;

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
			
			SQLiteManager sqLite( g_appPath + "/Octree.db" );
			sqLite.insertPoint( p0 );
			sqLite.insertPoint( p1 );
			sqLite.insertPoint( p2 );
			
			float epsilon = 1.e-15;
			PointPtr p = sqLite.getPoint( 0 );
			ASSERT_TRUE( p->equal( p0, epsilon ) );
			
			p = sqLite.getPoint( 1 );
			ASSERT_TRUE( p->equal( p1, epsilon ) );
			
			p = sqLite.getPoint( 2 ); 
			ASSERT_TRUE( p->equal( p2, epsilon ) );
		}
		
		TEST_F( SQLiteManagerTest, InsertAndGetIndexNodes )
		{
			using MortonCode = model::ShallowMortonCode;
			using Contents = vector< int >;
			using OctreeNode = model::ShallowOctreeNode;
			using OctreeNodePtr = model::ShallowOctreeNodePtr;
			using LeafNode = model::ShallowLeafNode< Contents >;
			using InnerNode = model::ShallowInnerNode< Contents >;
			using SQLiteManager = util::SQLiteManager< Point, MortonCode, OctreeNode >;
			
			MortonCode leafCode;
			leafCode.build( 2, 3, 4, 5 );
			LeafNode leafNode;
			Contents leafPoints = { 5, 4, 3, 2 };
			leafNode.setContents( leafPoints );
			
			SQLiteManager sqLite( g_appPath + "/Octree.db" );
			sqLite.insertNode< Contents >( leafCode, leafNode );
			
			MortonCode innerCode;
			innerCode.build( 1, 2, 3, 4 );
			InnerNode innerNode;
			Contents innerPoints = { 4, 3, 2, 1 };
			innerNode.setContents( innerPoints );
			
			sqLite.insertNode< Contents >( innerCode, innerNode );
			
			OctreeNodePtr queriedNode = sqLite.getNode< Contents >( leafCode );
			ASSERT_EQ( queriedNode->getContents< Contents >(), leafPoints );
			
			queriedNode = sqLite.getNode< Contents >( innerCode );
			ASSERT_EQ( queriedNode->getContents< Contents >(), innerPoints );
		}
		
		TEST_F( SQLiteManagerTest, InsertAndGetPointNodes )
		{
			using OctreeNode = model::ShallowOctreeNode;
			using OctreeNodePtr = model::ShallowOctreeNodePtr;
			using LeafNode = model::LeafNode< ShallowMortonCode, PointVector >;
			using SQLiteManager = util::SQLiteManager< Point, ShallowMortonCode, OctreeNode >;
			
			Point p0( vec3( 11.321565f, 4.658535f, 7.163479f ), vec3( 7.163479f, 4.658535f, 11.321565f ) );
			Point p1( vec3( 11.201763f, 5.635769f, 6.996898f ), vec3( 6.996898f, 5.635769f, 11.201763f ) );
			Point p2( vec3( 11.198129f, 4.750132f, 7.202037f ), vec3( 7.202037f, 4.750132f, 11.198129f ) );
			
			PointVector points;
			points.push_back( PointPtr( new Point( p0 ) ) );
			points.push_back( PointPtr( new Point( p1 ) ) );
			points.push_back( PointPtr( new Point( p2 ) ) );
			
			LeafNode node;
			node.setContents( points );
			
			ShallowMortonCode code;
			code.build( 1, 2, 3, 4 );
			SQLiteManager sqLite( g_appPath + "/Octree.db" );
			
			sqLite.insertNode< PointVector >( code, node );
			
			OctreeNodePtr queriedNode = sqLite.getNode< PointVector >( code );
			
			float epsilon = 1.e-15;
			PointVector queriedPoints = queriedNode->getContents< PointVector >();
			
			for( int i = 0; i < points.size(); ++i )
			{
				ASSERT_TRUE( points[ i ]->equal( *queriedPoints[ i ], epsilon ) );
			}
		}
		
		TEST_F( SQLiteManagerTest, InsertAndGetIdNodes )
		{
			using Contents = vector< int >;
			using OctreeNode = model::ShallowOctreeNode;
			using LeafNode = model::LeafNode< ShallowMortonCode, Contents >;
			using SQLiteManager = util::SQLiteManager< Point, ShallowMortonCode, OctreeNode >;
			using IdNode = model::IdNode< ShallowMortonCode >;
			
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
			
			SQLiteManager sqLite( g_appPath + "/Octree.db" );
			sqLite.insertNode< Contents >( code0, node0 );
			sqLite.insertNode< Contents >( code1, node1 );
			
			vector< IdNode > queried = sqLite.getIdNodes< Contents >( code0, intervalEnd );
			
			ShallowMortonCode queriedId = *queried[ 0 ].first;
			ASSERT_EQ( queriedId, code0 );
			
			Contents queriedInts = queried[ 0 ].second->getContents< Contents >();
			ASSERT_EQ( queriedInts, ints0 );
		}
		
		TEST_F( SQLiteManagerTest, DeleteNodes )
		{
			using MortonCode = model::ShallowMortonCode;
			using Contents = vector< int >;
			using OctreeNode = model::ShallowOctreeNode;
			using OctreeNodePtr = model::ShallowOctreeNodePtr;
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
			
			SQLiteManager sqLite( g_appPath + "/Octree.db" );
			sqLite.insertNode< Contents >( code0, node0 );
			sqLite.insertNode< Contents >( code1, node1 );
			sqLite.insertNode< Contents >( code2, node2 );
			sqLite.insertNode< Contents >( code3, node3 );
			
			sqLite.deleteNodes( code1, code2 );
			vector< OctreeNodePtr > queried = sqLite. template getNodes< Contents >( code0, code3 );
			
			ASSERT_EQ( queried.size(), 2 );
			ASSERT_EQ( queried[ 0 ]->getContents< Contents >(), Contents( 3, 0 ) );
			ASSERT_EQ( queried[ 1 ]->getContents< Contents >(), Contents( 3, 3 ) );
		}
		
		TEST_F( SQLiteManagerTest, InsertAndGetExtPointNodes )
		{
			using Point = ExtendedPoint;
			using PointPtr = ExtendedPointPtr;
			using PointVector = ExtendedPointVector;
			using OctreeNode = ShallowOctreeNode;
			using OctreeNodePtr = ShallowOctreeNodePtr;
			using LeafNode = model::LeafNode< ShallowMortonCode, PointVector >;
			using SQLiteManager = util::SQLiteManager< Point, ShallowMortonCode, OctreeNode >;

			Point p0( vec3( 0.000510f, 0.000549f, 0.000588f ), vec3( 0.13f, 0.14f, 0.15f ), vec3( 9.f, 10.f, 24.f ) );
			Point p1( vec3( 0.04f, 0.05f, 0.06f ), vec3( 0.04f, 0.05f, 0.06f ), vec3( 3.f, -31.f ,4.f ) );
			Point p2( vec3( 0.07f, 0.08f, 0.09f ), vec3( 0.07f, 0.08f, 0.09f ), vec3( -14.f, 5.f ,6.f ) );
			
			PointVector points;
			points.push_back( PointPtr( new Point( p0 ) ) );
			points.push_back( PointPtr( new Point( p1 ) ) );
			points.push_back( PointPtr( new Point( p2 ) ) );
			
			LeafNode node;
			node.setContents( points );
			
			ShallowMortonCode code;
			code.build( 1, 2, 3, 4 );
			SQLiteManager sqLite( g_appPath + "/Octree.db" );
			
			sqLite.insertNode< PointVector >( code, node );
			
			OctreeNodePtr queriedNode = sqLite.getNode< PointVector >( code );
			
			float epsilon = 1.e-15;
			PointVector queriedPoints = queriedNode->getContents< PointVector >();
			
			for( int i = 0; i < points.size(); ++i )
			{
				ASSERT_TRUE( points[ i ]->equal( *queriedPoints[ i ], epsilon ) );
			}
		}
		
		TEST_F( SQLiteManagerTest, InsertAndGetExtIdNodes )
		{
			using Contents = ExtendedPointVector;
			using LeafNode = model::LeafNode< ShallowMortonCode, Contents >;
			using SQLiteManager = util::SQLiteManager< ExtendedPoint, ShallowMortonCode, ShallowOctreeNode >;
			using IdNode = model::IdNode< ShallowMortonCode >;
			
			ExtendedPointPtr p0( new ExtendedPoint( vec3( 0.01f, 0.02f, 0.03f ), vec3( 0.01f, 0.02f, 0.03f ),
													vec3( 1.f, 15.f ,2.f ) ) );
			ExtendedPointPtr p1( new ExtendedPoint( vec3( 0.04f, 0.05f, 0.06f ), vec3( 0.04f, 0.05f, 0.06f ),
													vec3( 3.f, -31.f ,4.f ) ) );
			ExtendedPointPtr p2( new ExtendedPoint( vec3( 0.07f, 0.08f, 0.09f ), vec3( 0.07f, 0.08f, 0.09f ),
													vec3( -14.f, 5.f ,6.f ) ) );
			
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
			
			SQLiteManager sqLite( g_appPath + "/Octree.db" );
			sqLite.insertNode< Contents >( code0, node0 );
			sqLite.insertNode< Contents >( code1, node1 );
			
			vector< IdNode > queried = sqLite.getIdNodes< Contents >( code0, code0 );
			ASSERT_EQ( queried.size(), 1 );
			ASSERT_EQ( *queried[ 0 ].first, code0 );
			
			queried = sqLite.getIdNodes< Contents >( code0, intervalEnd );
			ASSERT_EQ( *queried[ 0 ].first, code0 );
			
			float epsilon = 1.e-15;
			Contents queriedVec = queried[ 0 ].second->getContents< Contents >();
			for( int i = 0; i < 3; ++i )
			{
				ASSERT_TRUE( vec0[ i ]->equal( *queriedVec[ i ], epsilon ) );
			}
		}
		
		template< typename Point, typename MortonCode, typename OctreeNode >
		void checkRequestResults( util::SQLiteManager< Point, MortonCode, OctreeNode >& sqLite,
								  const IdNode< MortonCode >& code0, const IdNode< MortonCode >& code1 )
		{
			using PointVector = vector< shared_ptr< Point > >;
			
			vector< IdNodeVector< MortonCode > > results = sqLite.getRequestResults( 10 );
	
			float epsilon = 1.e-15;
			for( IdNodeVector< MortonCode > queryResult : results )
			{
				int resultSize = queryResult.size();
				ASSERT_TRUE( resultSize == 1 || resultSize == 2 );
				
				if( resultSize == 1 )
				{
					ASSERT_EQ( *queryResult[ 0 ].first, *code0.first );
					
					PointVector expectedPoints = code0.second-> template getContents< PointVector >();
					PointVector queriedPoints = queryResult[ 0 ].second-> template getContents< PointVector >();;
					
					ASSERT_EQ( expectedPoints.size(), queriedPoints.size() );
					for( int i = 0; i < expectedPoints.size(); ++i )
					{
						ASSERT_TRUE( expectedPoints[ i ]->equal( *queriedPoints[ i ], epsilon ) );
					}
				}
				else
				{
					ASSERT_EQ( *queryResult[ 0 ].first, *code0.first );
					
					PointVector expectedPoints = code0.second-> template getContents< PointVector >();;
					PointVector queriedPoints = queryResult[ 0 ].second-> template getContents< PointVector >();;
					
					ASSERT_EQ( expectedPoints.size(), queriedPoints.size() );
					for( int i = 0; i < expectedPoints.size(); ++i )
					{
						ASSERT_TRUE( expectedPoints[ i ]->equal( *queriedPoints[ i ], epsilon ) );
					}
					
					expectedPoints = code1.second-> template getContents< PointVector >();;
					queriedPoints = queryResult[ 1 ].second-> template getContents< PointVector >();;
					
					ASSERT_EQ( expectedPoints.size(), queriedPoints.size() );
					for( int i = 0; i < expectedPoints.size(); ++i )
					{
						ASSERT_TRUE( expectedPoints[ i ]->equal( *queriedPoints[ i ], epsilon ) );
					}
				}
			}
		}
		
		TEST_F( SQLiteManagerTest, AsyncAPI )
		{
			using Contents = ExtendedPointVector;
			using LeafNode = model::LeafNode< ShallowMortonCode, Contents >;
			using LeafNodePtr = shared_ptr< LeafNode >;
			using SQLiteManager = util::SQLiteManager< ExtendedPoint, ShallowMortonCode, ShallowOctreeNode >;
			using IdNode = model::IdNode< ShallowMortonCode >;
			
			ExtendedPointPtr p0( new ExtendedPoint( vec3( 0.01f, 0.02f, 0.03f ), vec3( 0.01f, 0.02f, 0.03f ),
													vec3( 1.f, 15.f ,2.f ) ) );
			ExtendedPointPtr p1( new ExtendedPoint( vec3( 0.04f, 0.05f, 0.06f ), vec3( 0.04f, 0.05f, 0.06f ),
													vec3( 3.f, -31.f ,4.f ) ) );
			ExtendedPointPtr p2( new ExtendedPoint( vec3( 0.07f, 0.08f, 0.09f ), vec3( 0.07f, 0.08f, 0.09f ),
													vec3( -14.f, 5.f ,6.f ) ) );
			
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
			
			SQLiteManager sqLite( g_appPath + "/Octree.db" );
			sqLite.insertNode< Contents >( code0, node0 );
			sqLite.insertNode< Contents >( code1, node1 );
			
			ShallowMortonCodePtr code0Ptr( new ShallowMortonCode( code0 ) );
			ShallowMortonCodePtr code1Ptr( new ShallowMortonCode( code1 ) );
			
			IdNode idNode0( code0Ptr, LeafNodePtr( new LeafNode( node0 ) ) );
			IdNode idNode1( code1Ptr, LeafNodePtr( new LeafNode( node1 ) ) );
			
			default_random_engine generator;
			uniform_int_distribution< int > boolDistribution( 0, 1 );
			uniform_int_distribution< int > sleepDistribution( 0, 30 );
			
			int nRequests = 100;
			for( int i = 0; i < nRequests; ++i )
			{
				if( boolDistribution( generator ) )
				{
					sqLite.requestNodesAsync( ShallowMortonInterval( code0Ptr, code0Ptr ) );
				}
				else
				{
					sqLite.requestNodesAsync( ShallowMortonInterval( code0Ptr, code1Ptr ) );
				}
				
				this_thread::sleep_for( std::chrono::milliseconds( sleepDistribution( generator ) ) );
				
				if( boolDistribution( generator ) )
				{
					checkRequestResults( sqLite, idNode0, idNode1 );
				}
			}
			
			this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
			checkRequestResults( sqLite, idNode0, idNode1  );
		}
	}
}