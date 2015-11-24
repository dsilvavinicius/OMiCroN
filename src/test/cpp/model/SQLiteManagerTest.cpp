#include <gtest/gtest.h>
#include <random>
#include "SQLiteManager.h"
#include "MortonCode.h"
#include <MemoryManagerTypes.h>

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
			using SQLiteManager = util::SQLiteManager< Point, ShallowMortonCode, OctreeNode< PointVector > >;
			
			SPV_DefaultManager::initInstance( 1000000 );
			
			Point p0( vec3( 11.321565f, 4.658535f, 7.163479f ), vec3( 7.163479f, 4.658535f, 11.321565f ) );
			Point p1( vec3( 11.201763f, 5.635769f, 6.996898f ), vec3( 6.996898f, 5.635769f, 11.201763f ) );
			Point p2( vec3( 11.198129f, 4.750132f, 7.202037f ), vec3( 7.202037f, 4.750132f, 11.198129f ) );
			
			SQLiteManager sqLite( "Octree.db" );
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
			using Contents = IndexVector;
			using Node = model::OctreeNode< Contents >;
			using NodePtr = shared_ptr< Node >;
			using SQLiteManager = util::SQLiteManager< Point, MortonCode, Node >;
			
			SPI_DefaultManager::initInstance( 1000000 );
			
			MortonCode leafCode;
			leafCode.build( 2, 3, 4, 5 );
			Node leafNode( true );
			Contents leafPoints = { 5, 4, 3, 2 };
			leafNode.setContents( leafPoints );
			
			SQLiteManager sqLite( "Octree.db" );
			sqLite.insertNode( leafCode, leafNode );
			
			MortonCode innerCode;
			innerCode.build( 1, 2, 3, 4 );
			Node innerNode( false );
			Contents innerPoints = { 4, 3, 2, 1 };
			innerNode.setContents( innerPoints );
			
			sqLite.insertNode( innerCode, innerNode );
			
			NodePtr queriedNode = sqLite.getManagedNode( leafCode );
			ASSERT_EQ( queriedNode->getContents(), leafPoints );
			
			queriedNode = sqLite.getManagedNode( innerCode );
			ASSERT_EQ( queriedNode->getContents(), innerPoints );
		}
		
		TEST_F( SQLiteManagerTest, InsertAndGetPointNodes )
		{
			using Node = OctreeNode< PointVector >;
			using NodePtr = shared_ptr< Node >;
			using SQLiteManager = util::SQLiteManager< Point, ShallowMortonCode, Node >;
			
			SPV_DefaultManager::initInstance( 1000000 );
			
			Point p0( vec3( 11.321565f, 4.658535f, 7.163479f ), vec3( 7.163479f, 4.658535f, 11.321565f ) );
			Point p1( vec3( 11.201763f, 5.635769f, 6.996898f ), vec3( 6.996898f, 5.635769f, 11.201763f ) );
			Point p2( vec3( 11.198129f, 4.750132f, 7.202037f ), vec3( 7.202037f, 4.750132f, 11.198129f ) );
			
			PointVector points;
			points.push_back( makeManaged< Point >( p0 ) );
			points.push_back( makeManaged< Point >( p1 ) );
			points.push_back( makeManaged< Point >( p2 ) );
			
			Node node( true );
			node.setContents( points );
			
			ShallowMortonCode code;
			code.build( 1, 2, 3, 4 );
			SQLiteManager sqLite( "Octree.db" );
			
			sqLite.insertNode( code, node );
			
			NodePtr queriedNode = sqLite.getManagedNode( code );
			
			float epsilon = 1.e-15;
			PointVector queriedPoints = queriedNode->getContents();
			
			for( int i = 0; i < points.size(); ++i )
			{
				ASSERT_TRUE( points[ i ]->equal( *queriedPoints[ i ], epsilon ) );
			}
		}
		
		TEST_F( SQLiteManagerTest, InsertAndGetIdNodes )
		{
			using Contents = IndexVector;
			using Node = OctreeNode< Contents >;
			using SQLiteManager = util::SQLiteManager< Point, ShallowMortonCode, Node >;
			using IdNode = model::ManagedIdNode< ShallowMortonCode, Node >;
			
			SPI_DefaultManager::initInstance( 1000000 );
			
			int rawInts0[ 3 ] = { 1, 2, 3 };
			int rawInts1[ 3 ] = { 10, 20, 30 };
			Contents ints0( rawInts0, rawInts0 + 3 );
			Contents ints1( rawInts1, rawInts1 + 3 );
			
			Node node0( true );
			Node node1( true );
			node0.setContents( ints0 );
			node1.setContents( ints1 );
			
			ShallowMortonCode code0;
			code0.build( 1, 2, 3, 4 );
			ShallowMortonCode code1;
			code1.build( 7, 7, 7, 10 );
			ShallowMortonCode intervalEnd;
			intervalEnd.build( 1, 2, 3, 5 );
			
			SQLiteManager sqLite( "Octree.db" );
			sqLite.insertNode( code0, node0 );
			sqLite.insertNode( code1, node1 );
			
			SQLiteManager::ManagedIdNodeVector queried = sqLite.getManagedIdNodes( code0, intervalEnd );
			
			ShallowMortonCode queriedId = *queried[ 0 ].first;
			ASSERT_EQ( queriedId, code0 );
			
			Contents queriedInts = queried[ 0 ].second->getContents();
			ASSERT_EQ( queriedInts, ints0 );
		}
		
		TEST_F( SQLiteManagerTest, DeleteNodes )
		{
			using MortonCode = model::ShallowMortonCode;
			using Contents = IndexVector;
			using Node = OctreeNode< Contents >;
			using NodePtr = shared_ptr< Node >;
			using SQLiteManager = util::SQLiteManager< Point, MortonCode, Node >;
			
			SPI_DefaultManager::initInstance( 1000000 );
			
			MortonCode code0; code0.build( 0x8 );
			MortonCode code1; code1.build( 0x9 );
			MortonCode code2; code2.build( 0xA );
			MortonCode code3; code3.build( 0xB );
			
			Node node0( true ); node0.setContents( Contents( 3, 0 ) );
			Node node1( true ); node1.setContents( Contents( 3, 1 ) );
			Node node2( true ); node2.setContents( Contents( 3, 2 ) );
			Node node3( true ); node3.setContents( Contents( 3, 3 ) );
			
			SQLiteManager sqLite( "Octree.db" );
			sqLite.insertNode( code0, node0 );
			sqLite.insertNode( code1, node1 );
			sqLite.insertNode( code2, node2 );
			sqLite.insertNode( code3, node3 );
			
			sqLite.deleteNodes( code1, code2 );
			SQLiteManager::NodePtrVector queried = sqLite.getManagedNodes( code0, code3 );
			
			ASSERT_EQ( queried.size(), 2 );
			ASSERT_EQ( queried[ 0 ]->getContents(), Contents( 3, 0 ) );
			ASSERT_EQ( queried[ 1 ]->getContents(), Contents( 3, 3 ) );
		}
		
		TEST_F( SQLiteManagerTest, InsertAndGetExtPointNodes )
		{
			using Point = ExtendedPoint;
			using PointPtr = ExtendedPointPtr;
			using PointVector = ExtendedPointVector;
			using Node = OctreeNode< PointVector >;
			using NodePtr = shared_ptr< Node >;
			using SQLiteManager = util::SQLiteManager< Point, ShallowMortonCode, Node >;

			SEV_DefaultManager::initInstance( 1000000 );
			
			Point p0( vec3( 0.000510f, 0.000549f, 0.000588f ), vec3( 0.13f, 0.14f, 0.15f ), vec3( 9.f, 10.f, 24.f ) );
			Point p1( vec3( 0.04f, 0.05f, 0.06f ), vec3( 0.04f, 0.05f, 0.06f ), vec3( 3.f, -31.f ,4.f ) );
			Point p2( vec3( 0.07f, 0.08f, 0.09f ), vec3( 0.07f, 0.08f, 0.09f ), vec3( -14.f, 5.f ,6.f ) );
			
			PointVector points;
			points.push_back( makeManaged< Point >( p0 ) );
			points.push_back( makeManaged< Point >( p1 ) );
			points.push_back( makeManaged< Point >( p2 ) );
			
			Node node( true );
			node.setContents( points );
			
			ShallowMortonCode code;
			code.build( 1, 2, 3, 4 );
			SQLiteManager sqLite( "Octree.db" );
			
			sqLite.insertNode( code, node );
			
			NodePtr queriedNode = sqLite.getManagedNode( code );
			
			float epsilon = 1.e-15;
			PointVector queriedPoints = queriedNode->getContents();
			
			for( int i = 0; i < points.size(); ++i )
			{
				ASSERT_TRUE( points[ i ]->equal( *queriedPoints[ i ], epsilon ) );
			}
		}
		
		TEST_F( SQLiteManagerTest, InsertAndGetExtIdNodes )
		{
			using Contents = ExtendedPointVector;
			using Node = OctreeNode< Contents >;
			using SQLiteManager = util::SQLiteManager< ExtendedPoint, ShallowMortonCode, Node >;
			
			SEV_DefaultManager::initInstance( 1000000 );
			
			ExtendedPointPtr p0 = makeManaged< ExtendedPoint >( vec3( 0.01f, 0.02f, 0.03f ), vec3( 0.01f, 0.02f, 0.03f ),
																vec3( 1.f, 15.f ,2.f ) );
			ExtendedPointPtr p1 = makeManaged< ExtendedPoint >( vec3( 0.04f, 0.05f, 0.06f ), vec3( 0.04f, 0.05f, 0.06f ),
																vec3( 3.f, -31.f ,4.f ) );
			ExtendedPointPtr p2 = makeManaged< ExtendedPoint >( vec3( 0.07f, 0.08f, 0.09f ), vec3( 0.07f, 0.08f, 0.09f ),
																vec3( -14.f, 5.f ,6.f ) );
			
			ExtendedPointPtr rawPoints0[ 3 ] = { p0, p1, p2 };
			ExtendedPointPtr rawPoints1[ 3 ] = { p2, p1, p0 };
			
			Contents vec0( rawPoints0, rawPoints0 + 3 );
			Contents vec1( rawPoints1, rawPoints1 + 3 );
			
			Node node0( true );
			Node node1( true );
			node0.setContents( vec0 );
			node1.setContents( vec1 );
			
			ShallowMortonCode code0;
			code0.build( 1, 2, 3, 4 );
			ShallowMortonCode code1;
			code1.build( 7, 7, 7, 10 );
			ShallowMortonCode intervalEnd;
			intervalEnd.build( 1, 2, 3, 5 );
			
			SQLiteManager sqLite( "Octree.db" );
			sqLite.insertNode( code0, node0 );
			sqLite.insertNode( code1, node1 );
			
			SQLiteManager::ManagedIdNodeVector queried = sqLite.getManagedIdNodes( code0, code0 );
			ASSERT_EQ( queried.size(), 1 );
			ASSERT_EQ( *queried[ 0 ].first, code0 );
			
			queried = sqLite.getManagedIdNodes( code0, intervalEnd );
			ASSERT_EQ( *queried[ 0 ].first, code0 );
			
			float epsilon = 1.e-15;
			Contents queriedVec = queried[ 0 ].second->getContents();
			for( int i = 0; i < 3; ++i )
			{
				ASSERT_TRUE( vec0[ i ]->equal( *queriedVec[ i ], epsilon ) );
			}
		}
		
		template< typename Point, typename MortonCode, typename OctreeNode >
		void checkRequestResults( util::SQLiteManager< Point, MortonCode, OctreeNode >& sqLite,
								  const ManagedIdNode< MortonCode, OctreeNode >& code0,
							const ManagedIdNode< MortonCode, OctreeNode >& code1 )
		{
			using PointPtr = shared_ptr< Point >;
			using PointVector = vector< PointPtr, ManagedAllocator< PointPtr > >;
			using SQLite = util::SQLiteManager< Point, MortonCode, OctreeNode >;
			
			typename SQLite::QueryResultsVector results = sqLite.getRequestResults( 10 );
	
			float epsilon = 1.e-15;
			for( typename SQLite::ManagedIdNodeVector queryResult : results )
			{
				int resultSize = queryResult.size();
				ASSERT_TRUE( resultSize == 1 || resultSize == 2 );
				
				if( resultSize == 1 )
				{
					ASSERT_EQ( *queryResult[ 0 ].first, *code0.first );
					
					PointVector expectedPoints = code0.second->getContents();
					PointVector queriedPoints = queryResult[ 0 ].second->getContents();
					
					ASSERT_EQ( expectedPoints.size(), queriedPoints.size() );
					for( int i = 0; i < expectedPoints.size(); ++i )
					{
						ASSERT_TRUE( expectedPoints[ i ]->equal( *queriedPoints[ i ], epsilon ) );
					}
				}
				else
				{
					ASSERT_EQ( *queryResult[ 0 ].first, *code0.first );
					
					PointVector expectedPoints = code0.second->getContents();
					PointVector queriedPoints = queryResult[ 0 ].second->getContents();
					
					ASSERT_EQ( expectedPoints.size(), queriedPoints.size() );
					for( int i = 0; i < expectedPoints.size(); ++i )
					{
						ASSERT_TRUE( expectedPoints[ i ]->equal( *queriedPoints[ i ], epsilon ) );
					}
					
					expectedPoints = code1.second->getContents();
					queriedPoints = queryResult[ 1 ].second->getContents();
					
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
			using Node = OctreeNode< Contents >;
			using NodePtr = shared_ptr< Node >;
			using SQLiteManager = util::SQLiteManager< ExtendedPoint, ShallowMortonCode, Node >;
			using IdNode = model::ManagedIdNode< ShallowMortonCode, Node >;
			
			SEV_DefaultManager::initInstance( 1000000 );
			//SEV_Ken12MemoryManager::initInstance( 100000, 100000, 100000, 100000 );
			
			ExtendedPointPtr p0 = makeManaged< ExtendedPoint >( vec3( 0.01f, 0.02f, 0.03f ), vec3( 0.01f, 0.02f, 0.03f ),
																vec3( 1.f, 15.f ,2.f ) );
			ExtendedPointPtr p1 = makeManaged< ExtendedPoint >( vec3( 0.04f, 0.05f, 0.06f ), vec3( 0.04f, 0.05f, 0.06f ),
																vec3( 3.f, -31.f ,4.f ) );
			ExtendedPointPtr p2 = makeManaged< ExtendedPoint >( vec3( 0.07f, 0.08f, 0.09f ), vec3( 0.07f, 0.08f, 0.09f ),
																vec3( -14.f, 5.f ,6.f ) );
			
			//cout << "p1 ptr: " << p1.get() << endl << endl;
			
			ExtendedPointPtr rawPoints0[ 3 ] = { p0, p1, p2 };
			ExtendedPointPtr rawPoints1[ 3 ] = { p2, p1, p0 };
			
			Contents vec0( rawPoints0, rawPoints0 + 3 );
			Contents vec1( rawPoints1, rawPoints1 + 3 );
			
			NodePtr node0 = makeManaged< Node >( true );
			NodePtr node1 = makeManaged< Node >( true );
			node0->setContents( vec0 );
			node1->setContents( vec1 );
			
			ShallowMortonCode code0;
			code0.build( 1, 2, 3, 4 );
			ShallowMortonCode code1;
			code1.build( 7, 7, 7, 10 );
			
			SQLiteManager sqLite( "Octree.db" );
			sqLite.insertNode( code0, *node0 );
			sqLite.insertNode( code1, *node1 );
			
			ShallowMortonCodePtr code0Ptr = makeManaged< ShallowMortonCode >( code0 );
			ShallowMortonCodePtr code1Ptr = makeManaged< ShallowMortonCode > ( code1 );
			
			IdNode idNode0( code0Ptr, node0 );
			IdNode idNode1( code1Ptr, node1 );
			
			default_random_engine generator;
			uniform_int_distribution< int > boolDistribution( 0, 1 );
			uniform_int_distribution< int > sleepDistribution( 0, 30 );
			
			int nRequests = 100;
			for( int i = 0; i < nRequests; ++i )
			{
				if( boolDistribution( generator ) )
				{
					//cout << "request 0" << endl << endl;
					sqLite.requestNodesAsync( ShallowMortonInterval( code0Ptr, code0Ptr ) );
				}
				else
				{
					//cout << "request 1" << endl << endl;
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