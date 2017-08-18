#include "OctreeFile.h"

#include <gtest/gtest.h>

namespace model
{
	namespace test
	{
        class OctreeFileTest : public ::testing::Test
		{
		protected:
			void SetUp() {}
		};

		TEST_F( OctreeFileTest, WriteAndRead )
		{
			using Node = OctreeFile::Node;
			using NodePtr = OctreeFile::NodePtr;
			
			Surfel rootSurfel( Vec3( 0.0f, 0.1f, 0.2f ), Vec3( 0.3f, 0.4f, 0.5f ), Vec3( 0.6f, 0.7f, 0.8f ) );
			Surfel childSurfel( Vec3( 1.0f, 1.1f, 1.2f ), Vec3( 1.3f, 1.4f, 1.5f ), Vec3( 1.6f, 1.7f, 1.8f ) );
			Surfel grandChildSurfel( Vec3( 2.0f, 2.1f, 2.2f ), Vec3( 2.3f, 2.4f, 2.5f ), Vec3( 2.6f, 2.7f, 2.8f ) );
			
			Node root( Array< Surfel >( 1, rootSurfel ), false );
			{
				Node child( Array< Surfel >( 1, childSurfel ), false );
				Node grandChild( Array< Surfel >( 1, grandChildSurfel ), true );
				
				Array< Node > childChildren( 1 );
				childChildren[ 0 ] = std::move( grandChild );
				child.setChildren( std::move( childChildren ) );
				
				Array< Node > rootChildren( 1 );
				rootChildren[ 0 ] = std::move( child );
				root.setChildren( std::move( rootChildren ) );
			}
			
			OctreeFile::write( "test_octree.boc", root );
			
			NodePtr rootPtr = OctreeFile::read( "test_octree.boc" );
			
			ASSERT_EQ( rootPtr->getContents().size(), 1 );
			ASSERT_EQ( rootPtr->getContents()[ 0 ], rootSurfel );
			ASSERT_EQ( rootPtr->isLeaf(), false );
			ASSERT_EQ( rootPtr->child().size(), 1 );
			
			Node& child = rootPtr->child()[ 0 ];
			
			ASSERT_EQ( child.getContents().size(), 1 );
			ASSERT_EQ( child.getContents()[ 0 ], childSurfel );
			ASSERT_EQ( child.isLeaf(), false );
			ASSERT_EQ( child.child().size(), 1 );
			
			Node& grandChild = child.child()[ 0 ];
			
			ASSERT_EQ( grandChild.getContents().size(), 1 );
			ASSERT_EQ( grandChild.getContents()[ 0 ], grandChildSurfel );
			ASSERT_EQ( grandChild.isLeaf(), false );
			ASSERT_EQ( grandChild.child().size(), 0 );
		}
	}
}