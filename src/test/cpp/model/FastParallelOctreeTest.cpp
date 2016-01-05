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
				cout << "Node: " << expectedMorton.getPathToRoot( true ) << endl;
				
				ASSERT_EQ( parent, node.parent() );
				ASSERT_EQ( node.isLeaf(), node.child().size() == 0 );
				
				Dim expectedLvlDim( m_octree.dim(), expectedMorton.getLevel() );
				
				NodeContents contents = node.getContents();
				for( int i = 0; i < contents.size(); ++i )
				{
					Morton nodeMorton = expectedLvlDim.calcMorton( *contents[ i ] );
					ASSERT_EQ( expectedMorton, nodeMorton );
				}
			}
			
			void checkChild( Node& node, const vector< ulong >& expectedChildMortonBits ) const
			{
				if( !expectedChildMortonBits.empty() )
				{
					Morton nodeMorton; nodeMorton.build( expectedChildMortonBits[ 0 ] );
					Dim expectedLvlDim( m_octree.dim(), nodeMorton.traverseUp()->getLevel() );
					cout << "Checking child of " << expectedLvlDim.calcMorton( *node.getContents()[ 0 ] ).getPathToRoot( true );
				}
				
				NodeArray& child = node.child();
				ASSERT_EQ( expectedChildMortonBits.size(), child.size() );
				
				for( int i = 0; i < child.size(); ++i )
				{
					cout << "Child " << i << " node addr: " << child.data() + i << endl;
					checkNode( child[ i ], &node, expectedChildMortonBits[ i ] );
				}
				
				cout << endl;
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
			wrapper.checkChild( octree.root(), { 0xa, 0xc, 0xe } );
			
			// lvl 2
			wrapper.checkChild( octree.root().child()[ 0 ], { 0x53 } );
			wrapper.checkChild( octree.root().child()[ 1 ], { 0x61, 0x67  } );
			wrapper.checkChild( octree.root().child()[ 2 ], { 0x70, 0x71, 0x73, 0x76 } );
			
			// lvl 3
			wrapper.checkChild( octree.root().child()[ 0 ].child()[ 0 ], { 0x29b } );
			wrapper.checkChild( octree.root().child()[ 1 ].child()[ 0 ], { 0x30c } );
			wrapper.checkChild( octree.root().child()[ 1 ].child()[ 1 ], {} );
			wrapper.checkChild( octree.root().child()[ 2 ].child()[ 0 ], {} );
			wrapper.checkChild( octree.root().child()[ 2 ].child()[ 1 ], {} );
			wrapper.checkChild( octree.root().child()[ 2 ].child()[ 2 ], { 0x39d, 0x39f } );
			wrapper.checkChild( octree.root().child()[ 2 ].child()[ 3 ], { 0x3b0 } );
			
			// lvl 4
			wrapper.checkChild( octree.root().child()[ 0 ].child()[ 0 ].child()[ 0 ], { 0x14d8 } );
			wrapper.checkChild( octree.root().child()[ 1 ].child()[ 0 ].child()[ 0 ], { 0x1864 } );
			wrapper.checkChild( octree.root().child()[ 2 ].child()[ 2 ].child()[ 0 ], {} );
			wrapper.checkChild( octree.root().child()[ 2 ].child()[ 2 ].child()[ 1 ], {} );
			wrapper.checkChild( octree.root().child()[ 2 ].child()[ 3 ].child()[ 0 ], { 0x1d80, 0x1d82 } );
			
			// lvl 5
			wrapper.checkChild( octree.root().child()[ 0 ].child()[ 0 ].child()[ 0 ].child()[ 0 ], { 0xa6c0, 0xa6c3 } );
			wrapper.checkChild( octree.root().child()[ 1 ].child()[ 0 ].child()[ 0 ].child()[ 0 ], { 0xc320, 0xc325 } );
			wrapper.checkChild( octree.root().child()[ 2 ].child()[ 3 ].child()[ 0 ].child()[ 0 ], {} );
			wrapper.checkChild( octree.root().child()[ 2 ].child()[ 3 ].child()[ 0 ].child()[ 1 ], {} );
			
			// lvl 6
			wrapper.checkChild( octree.root().child()[ 0 ].child()[ 0 ].child()[ 0 ].child()[ 0 ].child()[ 0 ], {} );
			wrapper.checkChild( octree.root().child()[ 0 ].child()[ 0 ].child()[ 0 ].child()[ 0 ].child()[ 1 ], {} );
			wrapper.checkChild( octree.root().child()[ 1 ].child()[ 0 ].child()[ 0 ].child()[ 0 ].child()[ 0 ], {} );
			wrapper.checkChild( octree.root().child()[ 1 ].child()[ 0 ].child()[ 0 ].child()[ 0 ].child()[ 1 ], {} );
		}
		
		TEST( FastParallelOctree, Creation_MonoThread_Shallow_Point )
		{
			using Octree = FastParallelOctree< ShallowMortonCode, Point >;
			
			ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
			
			{
				Octree octree;
				octree.buildFromFile( "data/simple_point_octree.ply", 10 );
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
				OctreeTestWrapper< Octree > wrapper( octree );
				
				checkHierarchy( wrapper );
			}
			
			ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
		}
	}
}