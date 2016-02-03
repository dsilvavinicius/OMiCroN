#include <gtest/gtest.h>
#include "FastParallelOctree.h"

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
			// it is a sibling of the node at the same position at the line immediately above.
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
		
		TEST( FastParallelOctree, Creation_MonoThread_Shallow_Point )
		{
			using Octree = FastParallelOctree< ShallowMortonCode, Point >;
			
			ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
			
			{
				Octree octree;
				octree.buildFromFile( "data/simple_point_octree.ply", 10 );
				cout << octree << endl;
				
				OctreeTestWrapper< Octree > wrapper( octree );
				
				checkHierarchy( wrapper );
			}
			
			ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
		}
		
		TEST( FastParallelOctree, Creation_MonoThread_Shallow_Extended )
		{
			using Octree = FastParallelOctree< ShallowMortonCode, ExtendedPoint >;
			
			ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
			
			{
				Octree octree;
				octree.buildFromFile( "data/extended_point_octree.ply", 10 );
				cout << octree << endl;
				
				OctreeTestWrapper< Octree > wrapper( octree );
				
				checkHierarchy( wrapper );
			}
			
			ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
		}
		
		TEST( FastParallelOctree, Creation_MonoThread_Medium_Point )
		{
			using Octree = FastParallelOctree< MediumMortonCode, Point >;
			
			ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
			{
				Octree octree;
				octree.buildFromFile( "data/simple_point_octree.ply", 21 );
				cout << octree << endl;
				
				OctreeTestWrapper< Octree > wrapper( octree );
				
				checkHierarchy( wrapper );
			}
		}
		
		TEST( FastParallelOctree, Creation_MonoThread_Medium_Extended )
		{
			using Octree = FastParallelOctree< MediumMortonCode, ExtendedPoint >;
			
			ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
			
			{
				Octree octree;
				octree.buildFromFile( "data/extended_point_octree.ply", 21 );
				cout << octree << endl;
				
				OctreeTestWrapper< Octree > wrapper( octree );
				
				checkHierarchy( wrapper );
			}
			
			ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
		}
		
		TEST( FastParallelOctree, Creation_MultiThread_Shallow_Point )
		{
			using Octree = FastParallelOctree< ShallowMortonCode, Point >;
			
			ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
			
			{
				Octree octree;
				octree.buildFromFile( "data/simple_point_octree.ply", 10, 2 );
				cout << octree << endl;
				OctreeTestWrapper< Octree > wrapper( octree );
				
				checkHierarchy( wrapper );
			}
			
			ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
		}
		
		TEST( FastParallelOctree, Creation_MultiThread_SmallerWorklist_Release )
		{
			using Morton = ShallowMortonCode;
			using Octree = FastParallelOctree< Morton, Point >;
			using Sql = SQLiteManager< Point, Morton, Octree::Node >;
			using NodeArray = typename Sql::NodeArray;
			
			ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
			
			{
				Octree octree;
				octree.buildFromFile( "data/simple_point_octree.ply", 10, 3, 5000 );
				cout << octree << endl;
				
				// Nodes released are just in the database
 				Sql sql( "data/sorted_simple_point_octree.db", false );
 				Morton a; a.build( 0x1 );
 				Morton b = Morton::getLvlLast( 10 );
				
				sql.beginTransaction();
				NodeArray nodes = sql.getNodes( a, b );
				sql.endTransaction();
				
				ASSERT_EQ( 6, nodes.size() );
			}
			
			ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
		}
		
		TEST( FastParallelOctree, Creation_MultiThread_BiggerWorklist_Release )
		{
			using Morton = ShallowMortonCode;
			using Octree = FastParallelOctree< Morton, Point >;
			using Sql = SQLiteManager< Point, Morton, Octree::Node >;
			using NodeArray = typename Sql::NodeArray;
			
			ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
			
			{
				Octree octree;
				octree.buildFromFile( "data/simple_point_octree.ply", 10, 6, 5000 );
				cout << octree << endl;
				
				// Nodes released are just in the database
 				Sql sql( "data/sorted_simple_point_octree.db", false );
 				Morton a; a.build( 0x1 );
 				Morton b = Morton::getLvlLast( 10 );
				
				sql.beginTransaction();
				NodeArray nodes = sql.getNodes( a, b );
				sql.endTransaction();
				
				ASSERT_EQ( 8, nodes.size() );
			}
			
			ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
		}
		
		typedef struct TestParam
		{
			TestParam( const string& plyFilename, const int nThreads, const int hierarchyLvl, const int workItemSize,
					   const ulong memoryQuota )
			: m_nThreads( nThreads ),
			m_plyFilename( plyFilename ),
			m_hierarchyLvl( hierarchyLvl ),
			m_workItemSize( workItemSize ),
			m_memoryQuota( memoryQuota )
			{};
			
			friend ostream& operator<<( ostream &out, const TestParam &param );
			
			int m_nThreads;
			string m_plyFilename;
			int m_hierarchyLvl;
			int m_workItemSize;
			ulong m_memoryQuota;
		} TestParam;
		
		ostream& operator<<( ostream &out, const TestParam &param )
		{
			out << param.m_plyFilename << endl << "Threads: " << param.m_nThreads << endl << "Max lvl: "
				<< param.m_hierarchyLvl << endl << "Workitem size:" << param.m_workItemSize << endl
				<< "Mem quota (in bytes):" << param.m_memoryQuota << endl;
			return out;
		}
		
		class FastParallelOctreeStressTest
		: public ::testing::TestWithParam< TestParam >
		{};
		
		vector< TestParam > createTestParams()
		{
			vector< TestParam > param;
			
			for( int nThreads = 1; nThreads <= 8; ++nThreads )
			{
				for( int maxLvl = 5; maxLvl <= 20; maxLvl += 5 )
				{
					for( int worklistSize = 24; worklistSize <= 2048; worklistSize *= 2 )
					{
						for( ulong memQuota = 1024ul * 1024ul * 1024ul; memQuota <= 1024ul * 1024ul * 1024ul * 7;
							memQuota += 1024ul * 1024ul * 1024 )
						{
							param.push_back( TestParam( "../data/example/staypuff.ply", nThreads, maxLvl, worklistSize, memQuota ) );
							param.push_back( TestParam( "../../../src/data/real/tempietto_all.ply", nThreads, maxLvl, worklistSize, memQuota ) );
							param.push_back( TestParam( "../../../src/data/real/tempietto_sub_tot.ply", nThreads, maxLvl, worklistSize, memQuota ) );
						}
					}
				}
			}
			return param;
		}
		
		INSTANTIATE_TEST_CASE_P( FastParallelOctreeStressTest, FastParallelOctreeStressTest,
                        ::testing::ValuesIn( createTestParams() ) );
		
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
		
		TEST_P( FastParallelOctreeStressTest, Stress )
		{
			using Morton = MediumMortonCode;
			using Octree = FastParallelOctree< Morton, Point >;
			using OctreeDim = Octree::Dim;
			using Sql = SQLiteManager< Point, Morton, Octree::Node >;
			using NodeArray = typename Sql::NodeArray;
			
			ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
			
			{
				ofstream m_log( "FastParallelOctreeStressTest.log", ios_base::app );
				
				TestParam param = GetParam();
				m_log << "Starting building octree. Params:" << param << endl;
				auto start = Profiler::now( m_log );
				
				Octree octree;
				octree.buildFromFile( param.m_plyFilename, param.m_hierarchyLvl, param.m_workItemSize, param.m_memoryQuota );
				
				m_log << "Time to build octree (ms): " << Profiler::elapsedTime( start, m_log ) << endl << endl;
				
// 				string dbFilename = param.m_plyFilename;
// 				dbFilename.insert( dbFilename.find_last_of( '/' ) + 1, "sorted_" );
// 				dbFilename.replace( dbFilename.find_last_of( "." ), dbFilename.npos, ".db" );
// 				
// 				cout << "Sanity check..." << endl << endl;
// 				
// 				Sql sql( dbFilename, false );
// 				Morton rootCode; rootCode.build( 0x1 );
// 				checkNodeGeneral( octree.root(), rootCode, OctreeDim( octree.dim(), 0 ), sql );
			}
			
			ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
		}
	}
}