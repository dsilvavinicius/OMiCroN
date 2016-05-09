#include <gtest/gtest.h>
#include <MockRenderer.h>
#include "FastParallelOctree.h"
#include <StackTrace.h>
#include "FastParallelOctreeTestParam.h"

namespace model
{
	namespace test
	{
		template< typename Oct >
        class OctreeTestWrapper
        {
		public:
			using Octree = Oct;
			using Node = typename Octree::Node;
			using NodeArray = typename Node::NodeArray;
			using Morton = typename Octree::Morton;
			using Dim = typename Octree::Dim;
			using NodeContents = typename Node::ContentsArray;
			
			OctreeTestWrapper( Octree& octree )
			: m_octree( octree )
			{}
			
			void checkNode( const Node& node, const Node* parent, const ulong& expectedMortonBits ) const
			{
				Morton expectedMorton; expectedMorton.build( expectedMortonBits );
				SCOPED_TRACE( expectedMorton.getPathToRoot( true ) );
				
				// Debug
				{
					cout << "Checking node: " << expectedMorton.getPathToRoot( true );
					if( parent )
					{
						Dim parentLvlDim( m_octree.dim(), expectedMorton.getLevel() - 1 );
						cout << "Expected parent: " << parentLvlDim.calcMorton( *parent ).getPathToRoot( true )
							 << "Addr: " << parent << endl << endl;
					}
				}
				
				ASSERT_EQ( parent, node.parent() );
				ASSERT_EQ( node.isLeaf(), node.child().size() == 0 );
				
				Dim expectedLvlDim( m_octree.dim(), expectedMorton.getLevel() );
				
				const NodeContents& contents = node.getContents();
				for( int i = 0; i < contents.size(); ++i )
				{
					Morton nodeMorton = expectedLvlDim.calcMorton( *contents[ i ] );
					ASSERT_EQ( expectedMorton, nodeMorton );
				}
			}
			
			void checkChild( Node& node, const int nodeLvl, const vector< ulong >& expectedChildMortonBits ) const
			{
				// Debug
				{
					Dim nodeLvlDim( m_octree.dim(), nodeLvl );
					cout << "Checking children of " << nodeLvlDim.calcMorton( node ).getPathToRoot( true ) << endl;
				}
				
				NodeArray& child = node.child();
				ASSERT_EQ( expectedChildMortonBits.size(), child.size() );
				
				for( int i = 0; i < child.size(); ++i )
				{
					checkNode( child[ i ], &node, expectedChildMortonBits[ i ] );
				}
			}
			
			Octree& m_octree;
		};
		
		template< typename OctreeTestWrapper >
		void checkHierarchy( OctreeTestWrapper& wrapper )
		{
			using Octree = typename OctreeTestWrapper::Octree;
			
			// Expected hierarchy. 0x1 is the root node. A node with an arrow that points to nothing means that
			// it is a sibling of the node at the same position at the text line immediately above.
			//
			// 0xa6c0 -> 0x14d8 -> 0x29b -> 0x53 -> 0xa -> 0x1
			// 0xa6c3 -> 
			// 0xc320 -> 0x1864 -> 0x30c -> 0x61 -> 0xc ->
			// 0xc325 ->
			//								0x67 -> 
			
			//								0x70 -> 0xe ->
			//							    0x71 ->
			//					   0x39d -> 0x73 ->
			//					   0x39f ->
			//			 0x1d80 -> 0x3b0 -> 0x76 ->
			//			 0x1d82 ->
			
			Octree& octree = wrapper.m_octree;
			
			// lvl 0
			wrapper.checkNode( octree.root(), nullptr, 0x1 );
			
			// lvl 1
			wrapper.checkChild( octree.root(), 0, { 0xa, 0xc, 0xe } );
			
			// lvl 2
			wrapper.checkChild( octree.root().child()[ 0 ], 1, { 0x53 } );
			wrapper.checkChild( octree.root().child()[ 1 ], 1, { 0x61, 0x67  } );
			wrapper.checkChild( octree.root().child()[ 2 ], 1, { 0x70, 0x71, 0x73, 0x76 } );
			
			// lvl 3
			wrapper.checkChild( octree.root().child()[ 0 ].child()[ 0 ], 2, { 0x29b } );
			wrapper.checkChild( octree.root().child()[ 1 ].child()[ 0 ], 2, { 0x30c } );
			wrapper.checkChild( octree.root().child()[ 1 ].child()[ 1 ], 2, {} );
			wrapper.checkChild( octree.root().child()[ 2 ].child()[ 0 ], 2, {} );
			wrapper.checkChild( octree.root().child()[ 2 ].child()[ 1 ], 2, {} );
			wrapper.checkChild( octree.root().child()[ 2 ].child()[ 2 ], 2, { 0x39d, 0x39f } );
			wrapper.checkChild( octree.root().child()[ 2 ].child()[ 3 ], 2, { 0x3b0 } );
			
			// lvl 4
			wrapper.checkChild( octree.root().child()[ 0 ].child()[ 0 ].child()[ 0 ], 3, { 0x14d8 } );
			wrapper.checkChild( octree.root().child()[ 1 ].child()[ 0 ].child()[ 0 ], 3, { 0x1864 } );
			wrapper.checkChild( octree.root().child()[ 2 ].child()[ 2 ].child()[ 0 ], 3, {} );
			wrapper.checkChild( octree.root().child()[ 2 ].child()[ 2 ].child()[ 1 ], 3, {} );
			wrapper.checkChild( octree.root().child()[ 2 ].child()[ 3 ].child()[ 0 ], 3, { 0x1d80, 0x1d82 } );
			
			// lvl 5
			wrapper.checkChild( octree.root().child()[ 0 ].child()[ 0 ].child()[ 0 ].child()[ 0 ], 4, { 0xa6c0, 0xa6c3 } );
			wrapper.checkChild( octree.root().child()[ 1 ].child()[ 0 ].child()[ 0 ].child()[ 0 ], 4, { 0xc320, 0xc325 } );
			wrapper.checkChild( octree.root().child()[ 2 ].child()[ 3 ].child()[ 0 ].child()[ 0 ], 4, {} );
			wrapper.checkChild( octree.root().child()[ 2 ].child()[ 3 ].child()[ 0 ].child()[ 1 ], 4, {} );
			
			// lvl 6
			wrapper.checkChild( octree.root().child()[ 0 ].child()[ 0 ].child()[ 0 ].child()[ 0 ].child()[ 0 ], 5, {} );
			wrapper.checkChild( octree.root().child()[ 0 ].child()[ 0 ].child()[ 0 ].child()[ 0 ].child()[ 1 ], 5, {} );
			wrapper.checkChild( octree.root().child()[ 1 ].child()[ 0 ].child()[ 0 ].child()[ 0 ].child()[ 0 ], 5, {} );
			wrapper.checkChild( octree.root().child()[ 1 ].child()[ 0 ].child()[ 0 ].child()[ 0 ].child()[ 1 ], 5, {} );
		}
		
		template< typename Octree >
		void waitAsynCreation( Octree& octree )
		{
			MockRenderer renderer;
			while( !octree.isCreationFinished() )
			{
				octree.trackFront( renderer, 0.f );
			}
		}
		
		/** Check expectations of octree for the in-core tests. */
		template< typename Octree >
		void testInCore( Octree& octree )
		{
			waitAsynCreation( octree );
			cout << octree << endl;
			
			OctreeTestWrapper< Octree > wrapper( octree );
			
			checkHierarchy( wrapper );
		}
		
		TEST( FastParallelOctreeTest, Creation_MonoThread_Shallow_Point )
		{
			using Octree = FastParallelOctree< ShallowMortonCode, Point >;
			
			ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
			
			{
				Octree octree( "data/simple_point_octree.ply", 10 );
				testInCore( octree );
			}
			
			ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
		}
		
		TEST( FastParallelOctreeTest, Creation_MonoThread_Shallow_Extended )
		{
			using Octree = FastParallelOctree< ShallowMortonCode, ExtendedPoint >;
			
			ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
			
			{
				Octree octree( "data/extended_point_octree.ply", 10 );
				testInCore( octree );
			}
			
			ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
		}
		
		TEST( FastParallelOctreeTest, Creation_MonoThread_Medium_Point )
		{
			using Octree = FastParallelOctree< MediumMortonCode, Point >;
			
			ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
			{
				Octree octree( "data/simple_point_octree.ply", 21 );
				testInCore( octree );
			}
		}
		
		TEST( FastParallelOctreeTest, Creation_MonoThread_Medium_Extended )
		{
			using Octree = FastParallelOctree< MediumMortonCode, ExtendedPoint >;
			
			ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
			
			{
				Octree octree( "data/extended_point_octree.ply", 21 );
				testInCore( octree );
			}
			
			ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
		}
		
		TEST( FastParallelOctreeTest, Creation_MultiThread_Shallow_Point )
		{
			using Octree = FastParallelOctree< ShallowMortonCode, Point >;
			
			ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
			
			{
				Octree octree( "data/simple_point_octree.ply", 10, 2 );
				testInCore( octree );
			}
			
			ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
		}
		
		TEST( FastParallelOctreeTest, Creation_MultiThread_SmallerWorklist_Release )
		{
			using Morton = ShallowMortonCode;
			using Octree = FastParallelOctree< Morton, Point >;
			using Sql = SQLiteManager< Point, Morton, Octree::Node >;
			using NodeArray = typename Sql::NodeArray;
			
			ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
			
			{
				Octree octree( "data/simple_point_octree.ply", 10, 3, 1000 );
				waitAsynCreation( octree );
				
				cout << octree << endl;
				
				// Nodes released are just in the database
 				Sql sql( "data/sorted_simple_point_octree.db", false );
 				Morton a; a.build( 0x1 );
 				Morton b = Morton::getLvlLast( 10 );
				
				sql.beginTransaction();
				NodeArray nodes = sql.getNodes( a, b );
				sql.endTransaction();
				
				cout << "Database size: " << nodes.size() << endl << endl;
			}
			
			ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
		}
		
		TEST( FastParallelOctreeTest, Creation_MultiThread_BiggerWorklist_Release )
		{
			using Morton = ShallowMortonCode;
			using Octree = FastParallelOctree< Morton, Point >;
			using Sql = SQLiteManager< Point, Morton, Octree::Node >;
			using NodeArray = typename Sql::NodeArray;
			
			ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
			
			{
				Octree octree( "data/simple_point_octree.ply", 10, 6, 1000 );
				waitAsynCreation( octree );
				
				cout << octree << endl;
				
				// Nodes released are just in the database
 				Sql sql( "data/sorted_simple_point_octree.db", false );
 				Morton a; a.build( 0x1 );
 				Morton b = Morton::getLvlLast( 10 );
				
				sql.beginTransaction();
				NodeArray nodes = sql.getNodes( a, b );
				sql.endTransaction();
				
				cout << "Database size: " << nodes.size() << endl << endl;
			}
			
			ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
		}
		
		template< typename Node, typename Morton, typename OctreeDim, typename Sql >
		void checkNodeGeneral( Node& node, const Morton& expectedMorton, const OctreeDim& dim, Sql& sql )
		{
			SCOPED_TRACE( expectedMorton.getPathToRoot( true ) );
			
			ASSERT_EQ( expectedMorton, dim.calcMorton( node ) );
			
			if( node.isLeaf() )
			{
				ASSERT_TRUE( node.child().empty() );
			}
			else
			{
				typename Node::NodeArray& children = node.child();
				
				bool fromDb = false;
				OctreeDim nextLvlDim( dim, dim.m_nodeLvl + 1 );
				pair< shared_ptr< Morton >, shared_ptr< Morton > > childInterval = expectedMorton.getChildInterval();
				
				if( children.size() == 0 )
				{
					fromDb = true;
					children = sql.getNodes( *childInterval.first, *childInterval.second );
				}
				
				ASSERT_NE( 0, children.size() );
					
				for( int i = 0; i < children.size(); ++i )
				{
					if( !fromDb )
					{
						ASSERT_EQ( &node, children[ i ].parent() );
					}
					
					Morton childMorton = nextLvlDim.calcMorton( children[ i ] );
					if( i < children.size() - 1 )
					{
						ASSERT_LT( childMorton, nextLvlDim.calcMorton( children[ i + 1 ] ) );
					}
					ASSERT_LE( *childInterval.first, childMorton );
					ASSERT_LE( childMorton, *childInterval.second );
					
					checkNodeGeneral( children[ i ], childMorton, nextLvlDim, sql );
				}
			}
		}
		
		void testSanity( const FastParallelOctreeTestParam& params )
		{
			using Morton = MediumMortonCode;
			using Octree = FastParallelOctree< Morton, Point >;
			using OctreeDim = Octree::Dim;
			using Sql = SQLiteManager< Point, Morton, Octree::Node >;
			
			Octree octree( params.m_plyFilename, params.m_hierarchyLvl, params.m_workItemSize, params.m_memoryQuota,
						params.m_nThreads );
			waitAsynCreation( octree );
			
			string dbFilename = params.m_plyFilename;
			dbFilename.insert( dbFilename.find_last_of( '/' ) + 1, "sorted_" );
			dbFilename.replace( dbFilename.find_last_of( "." ), dbFilename.npos, ".db" );
			
			cout << "Sanity check..." << endl << dbFilename << endl << endl;
				
			Sql sql( dbFilename, false );
			Morton rootCode; rootCode.build( 0x1 );
			checkNodeGeneral( octree.root(), rootCode, OctreeDim( octree.dim(), 0 ), sql );
		}
		
		TEST( FastParallelOctreeTest, Creation_Shallow_StayPuff_Sanity )
		{
			FastParallelOctreeTestParam params( "../../../src/data/example/staypuff.ply", 4, 4, 16, 10ul * 1024ul * 1024ul );
			testSanity( params );
		}
		
		TEST( FastParallelOctreeTest, Creation_StayPuff_Sanity )
		{
			FastParallelOctreeTestParam params( "../../../src/data/example/staypuff.ply", 4, 20, 1024, 10ul * 1024ul * 1024ul );
			testSanity( params );
		}
		
		TEST( FastParallelOctreeTest, Creation_Prova5M_Sanity )
		{
			FastParallelOctreeTestParam params( "../../../src/data/real/prova5M.ply", 4, 20, 1024, 10ul * 1024ul * 1024ul );
			testSanity( params );
		}
		
		TEST( FastParallelOctreeTest, Creation_Prova10M_Sanity )
		{
			FastParallelOctreeTestParam params( "../../../src/data/real/prova10M.ply", 4, 20, 1024, 10ul * 1024ul * 1024ul );
			testSanity( params );
		}
		
		TEST( FastParallelOctreeTest, Creation_TempiettoAll_Sanity )
		{
			FastParallelOctreeTestParam params( "../../../src/data/real/tempietto_all.ply", 4, 20, 1024, 10ul * 1024ul * 1024ul );
			testSanity( params );
		}
		
		TEST( FastParallelOctreeTest, Creation_TempiettoSub_Sanity )
		{
			FastParallelOctreeTestParam params( "../../../src/data/real/tempietto_sub_tot.ply", 4, 20,
												1024, 10ul * 1024ul * 1024ul );
			testSanity( params );
		}
	}
}