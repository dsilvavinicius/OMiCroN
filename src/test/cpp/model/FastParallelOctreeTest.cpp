#include <gtest/gtest.h>
#include "FastParallelOctree.h"

namespace model
{
	namespace test
	{
		template< typename Octree >
        class FastParallelOctreeTest
        :  public ::testing::Test
        {
		public:
			using Node = typename Octree::Node;
			using NodeArray = typename Node::NodeArray;
			using Morton = typename Octree::Morton;
			using Dim = typename Octree::Dim;
			using NodeContents = typename Node::ContentsArray;
			
			FastParallelOctreeTest()
			{
				init();
			}
			
			void init();
			
			void checkNode( const Node& node, const Node* parent, const ulong& expectedMortonBits ) const
			{
				ASSERT_EQ( parent, node.parent() );
				ASSERT_EQ( node.isLeaf(), node.child().size() == 0 );
				
				Morton expectedMorton; expectedMorton.build( expectedMortonBits );
				SCOPED_TRACE( expectedMorton.getPathToRoot( true ) );
				
				Dim octreeDim = m_octree.dim();
				Dim expectedLvlDim( octreeDim.m_origin, octreeDim.m_size, expectedMorton.getLevel() );
				
				NodeContents contents = node.getContents();
				for( int i = 0; i < contents.size(); ++i )
				{
					Morton nodeMorton = expectedLvlDim.calcMorton( *contents[ i ] );
					ASSERT_EQ( expectedMorton, nodeMorton );
				}
			}
			
			void checkChild( const Node& node, const vector< ulong >& expectedChildMortonBits ) const
			{
				const NodeArray& child = node.child();
				ASSERT_EQ( expectedChildMortonBits.size(), child.size() );
				
				for( int i = 0; i < child.size(); ++i )
				{
					checkNode( child[ i ], &node, expectedChildMortonBits[ i ] );
				}
			}
			
			Octree m_octree;
		};
		
		template<>
        void FastParallelOctreeTest< FastParallelOctree< ShallowMortonCode, Point > >::init()
		{
			m_octree.buildFromFile( "data/simple_point_octree.ply", 10 );
		}
		
		template<>
        void FastParallelOctreeTest< FastParallelOctree< ShallowMortonCode, ExtendedPoint > >::init()
		{
			m_octree.buildFromFile( "data/simple_point_octree.ply", 10 );
		}
		
		template<>
        void FastParallelOctreeTest< FastParallelOctree< MediumMortonCode, Point > >::init()
		{
			m_octree.buildFromFile( "data/simple_point_octree.ply", 20 );
		}
		
		template<>
        void FastParallelOctreeTest< FastParallelOctree< MediumMortonCode, ExtendedPoint > >::init()
		{
			m_octree.buildFromFile( "data/simple_point_octree.ply", 20 );
		}
		
		using testing::Types;
		
		typedef Types<  FastParallelOctree< ShallowMortonCode, Point >,
						FastParallelOctree< ShallowMortonCode, ExtendedPoint >,
						FastParallelOctree< MediumMortonCode, Point >,
						FastParallelOctree< MediumMortonCode, ExtendedPoint >
						> Implementations;
		
		TYPED_TEST_CASE( FastParallelOctreeTest, Implementations );
		
		TYPED_TEST( FastParallelOctreeTest, Creation )
		{
			// Expected hierarchy. 0x1 is the root node. A node with an arrow that points to nothing means that
			// it is a sibling of the node at the same position at the line immediately above.
			//
			// 0xa6c3 -> 0x14d8 -> 0x29b -> 0x53 -> 0xa -> 0x1
			// 0xa6c0 -> 
			//								0x67 -> 0xc ->
			// 0xc325 -> 0x1864 -> 0x30c -> 0x61 ->
			// 0xc320 ->
			//								0x70 -> 0xe ->
			//							    0x71 ->
			//					   0x39f -> 0x73 ->
			//					   0x39d ->
			//			 0x1d82 -> 0x3b0 -> 0x76 ->
			//			 0x1d80 ->
			
			// lvl 0
			this->checkNode( this->m_octree.root(), nullptr, 0x1 );
			
			// lvl 1
			this->checkChild( this->m_octree.root(), { 0xa, 0xc, 0xe } );
			
			// lvl 2
			this->checkChild( this->m_octree.root().child()[ 0 ], { 0x53 } );
			this->checkChild( this->m_octree.root().child()[ 1 ], { 0x67, 0x61 } );
			this->checkChild( this->m_octree.root().child()[ 2 ], { 0x70, 0x71, 0x73, 0x76 } );
			
			// lvl 3
			this->checkChild( this->m_octree.root().child()[ 0 ].child()[ 0 ], { 0x29b } );
			this->checkChild( this->m_octree.root().child()[ 1 ].child()[ 0 ], {} );
			this->checkChild( this->m_octree.root().child()[ 1 ].child()[ 1 ], { 0x30c } );
			this->checkChild( this->m_octree.root().child()[ 2 ].child()[ 0 ], {} );
			this->checkChild( this->m_octree.root().child()[ 2 ].child()[ 1 ], {} );
			this->checkChild( this->m_octree.root().child()[ 2 ].child()[ 2 ], { 0x39f, 0x39d } );
			this->checkChild( this->m_octree.root().child()[ 2 ].child()[ 3 ], { 0x3b0 } );
			
			// lvl 4
			this->checkChild( this->m_octree.root().child()[ 0 ].child()[ 0 ].child()[ 0 ], { 0x14d8 } );
			this->checkChild( this->m_octree.root().child()[ 1 ].child()[ 1 ].child()[ 0 ], { 0x1864 } );
			this->checkChild( this->m_octree.root().child()[ 2 ].child()[ 2 ].child()[ 0 ], {} );
			this->checkChild( this->m_octree.root().child()[ 2 ].child()[ 2 ].child()[ 1 ], {} );
			this->checkChild( this->m_octree.root().child()[ 2 ].child()[ 3 ].child()[ 0 ], { 0x1d82, 0x1d80 } );
			
			// lvl 5
			this->checkChild( this->m_octree.root().child()[ 0 ].child()[ 0 ].child()[ 0 ].child()[ 0 ], { 0xa6c3, 0xa6c0 } );
			this->checkChild( this->m_octree.root().child()[ 1 ].child()[ 1 ].child()[ 0 ].child()[ 0 ], { 0xc325, 0xc320 } );
			this->checkChild( this->m_octree.root().child()[ 2 ].child()[ 3 ].child()[ 0 ].child()[ 0 ], {} );
			this->checkChild( this->m_octree.root().child()[ 2 ].child()[ 3 ].child()[ 0 ].child()[ 1 ], {} );
			
			// lvl 6
			this->checkChild( this->m_octree.root().child()[ 0 ].child()[ 0 ].child()[ 0 ].child()[ 0 ].child()[ 0 ], {} );
			this->checkChild( this->m_octree.root().child()[ 0 ].child()[ 0 ].child()[ 0 ].child()[ 0 ].child()[ 1 ], {} );
			this->checkChild( this->m_octree.root().child()[ 1 ].child()[ 1 ].child()[ 0 ].child()[ 0 ].child()[ 0 ], {} );
			this->checkChild( this->m_octree.root().child()[ 1 ].child()[ 1 ].child()[ 0 ].child()[ 0 ].child()[ 1 ], {} );
		}
	}
}