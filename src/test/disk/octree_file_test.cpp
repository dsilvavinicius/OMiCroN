#include "omicron/disk/octree_file.h"

#include <gtest/gtest.h>

namespace omicron::test::disk
{
    using namespace basic;
    
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
        
        Surfel childSurfel0( Vec3( 1.0f, 1.1f, 1.2f ), Vec3( 1.3f, 1.4f, 1.5f ), Vec3( 1.6f, 1.7f, 1.8f ) );
        Surfel childSurfel1( Vec3( 1.9f, 1.10f, 1.11f ), Vec3( 1.12f, 1.13f, 1.14f ), Vec3( 1.15f, 1.16f, 1.17f ) );
        
        Surfel grandChildSurfel0( Vec3( 2.0f, 2.1f, 2.2f ), Vec3( 2.3f, 2.4f, 2.5f ), Vec3( 2.6f, 2.7f, 2.8f ) );
        Surfel grandChildSurfel1( Vec3( 2.9f, 2.10f, 2.11f ), Vec3( 2.12f, 2.13f, 2.14f ), Vec3( 2.15f, 2.16f, 2.17f ) );
        
        Node root( Array< Surfel >( 1, rootSurfel ), false );
        {
            Array< Surfel > childSurfels( 2 );
            childSurfels[ 0 ] = childSurfel0;
            childSurfels[ 1 ] = childSurfel1;
            
            Node child( childSurfels, false );
            
            Array< Surfel > grandChildSurfels( 2 );
            grandChildSurfels[ 0 ] = grandChildSurfel0;
            grandChildSurfels[ 1 ] = grandChildSurfel1;
            Node grandChild( grandChildSurfels, true );
            
            Array< Node > childChildren( 1 );
            childChildren[ 0 ] = std::move( grandChild );
            child.setChildren( std::move( childChildren ) );
            
            Array< Node > rootChildren( 1 );
            rootChildren[ 0 ] = std::move( child );
            root.setChildren( std::move( rootChildren ) );
        }
        
        OctreeFile::writeDepth( "test_octree.boc", root );
        
        NodePtr rootPtr = OctreeFile::read( "test_octree.boc" );
        
        ASSERT_EQ( rootPtr->parent(), nullptr );
        ASSERT_EQ( rootPtr->getContents().size(), 1 );
        ASSERT_EQ( rootPtr->getContents()[ 0 ], rootSurfel );
        ASSERT_EQ( rootPtr->isLeaf(), false );
        ASSERT_EQ( rootPtr->child().size(), 1 );
        
        Node& child = rootPtr->child()[ 0 ];
        
        ASSERT_EQ( child.parent(), rootPtr.get() );
        ASSERT_EQ( child.getContents().size(), 2 );
        ASSERT_EQ( child.getContents()[ 0 ], childSurfel0 );
        ASSERT_EQ( child.getContents()[ 1 ], childSurfel1 );
        ASSERT_EQ( child.isLeaf(), false );
        ASSERT_EQ( child.child().size(), 1 );
        
        Node& grandChild = child.child()[ 0 ];
        
        ASSERT_EQ( grandChild.parent(), &child );
        ASSERT_EQ( grandChild.getContents().size(), 2 );
        ASSERT_EQ( grandChild.getContents()[ 0 ], grandChildSurfel0 );
        ASSERT_EQ( grandChild.getContents()[ 1 ], grandChildSurfel1 );
        ASSERT_EQ( grandChild.isLeaf(), true );
        ASSERT_EQ( grandChild.child().size(), 0 );
    }
}
