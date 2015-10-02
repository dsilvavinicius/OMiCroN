#include <gtest/gtest.h>
#include "Serializer.h"
#include <OctreeNode.h>
#include <MemoryManagerTypes.h>

namespace model
{
	namespace test
	{
        class SerializerTest
        : public ::testing::Test
		{
		protected:
			void SetUp(){}
		};
		
		TEST_F( SerializerTest, PointVector )
		{
			SPV_BitMapMemoryManager::initInstance( 1000000 );
			
			Point p0( vec3( 11.321565f, 4.658535f, 7.163479f ), vec3( 7.163479f, 4.658535f, 11.321565f ) );
			Point p1( vec3( 11.201763f, 5.635769f, 6.996898f ), vec3( 6.996898f, 5.635769f, 11.201763f ) );
			Point p2( vec3( 11.198129f, 4.750132f, 7.202037f ), vec3( 7.202037f, 4.750132f, 11.198129f ) );
			
			PointPtr pointArray[3] = { PointPtr( new Point( p0 ) ), PointPtr( new Point( p1 ) ), PointPtr( new Point( p2 ) ) };
			PointVector points( pointArray, pointArray + 3 );
			
			byte* bytes;
			Serializer::serialize( points, &bytes );
			
			PointVector deserializedPoints;
			Serializer::deserialize( bytes, deserializedPoints );
			
			float epsilon = 1.e-15;
			for( int i = 0; i < points.size(); ++i )
			{
				ASSERT_TRUE( points[ i ]->equal( *deserializedPoints[ i ], epsilon ) );
			}
			
			Serializer::dispose( bytes );
		}
		
		TEST_F( SerializerTest, ExtendedPointVector )
		{
			using Point = ExtendedPoint;
			using PointPtr = ExtendedPointPtr;
			using PointVector = ExtendedPointVector;
			
			SEV_BitMapMemoryManager::initInstance( 1000000 );
			
			PointPtr p0( new Point( vec3( 0.01f, 0.02f, 0.03f ), vec3( 0.01f, 0.02f, 0.03f ), vec3( 1.f, 15.f ,2.f ) ) );
			PointPtr p1( new Point( vec3( 0.04f, 0.05f, 0.06f ), vec3( 0.04f, 0.05f, 0.06f ), vec3( 3.f, -31.f ,4.f ) ) );
			PointPtr p2( new Point( vec3( 0.07f, 0.08f, 0.09f ), vec3( 0.07f, 0.08f, 0.09f ), vec3( -14.f, 5.f ,6.f ) ) );
			
			PointPtr pointArray[3] = { p0, p1, p2 };
			PointVector points( pointArray, pointArray + 3 );
			
			byte* bytes;
			Serializer::serialize( points, &bytes );
			
			PointVector deserializedPoints;
			Serializer::deserialize( bytes, deserializedPoints );
			
			float epsilon = 1.e-15;
			for( int i = 0; i < points.size(); ++i )
			{
				ASSERT_TRUE( points[ i ]->equal( *deserializedPoints[ i ], epsilon ) );
			}
			Serializer::dispose( bytes );
		}
		
		TEST_F( SerializerTest, LeafIndexNode )
		{
			using Contents = vector< uint >;
			using Node = LeafNode< Contents >;
			
			SPI_BitMapMemoryManager::initInstance( 1000000 );
			
			Node node;
			uint array[ 3 ] = { 1, 2, 3 };
			Contents contents( array, array + 3 );
			node.setContents( contents );
			
			byte* bytes;
			
			node.serialize< Contents >( &bytes );
			auto nodePtr = ( Node* ) OctreeNode::deserialize< Contents >( bytes );
			Contents resultContents = nodePtr->getContents();
			
			ASSERT_EQ( resultContents, contents );
			Serializer::dispose( bytes );
			delete nodePtr;
		}
		
		TEST_F( SerializerTest, InnerIndexNode )
		{
			using Contents = vector< uint >;
			using Node = InnerNode< Contents >;
			
			SPI_BitMapMemoryManager::initInstance( 1000000 );
			
			Node node;
			uint array[ 3 ] = { 1, 2, 3 };
			Contents contents( array, array + 3 );
			node.setContents( contents );
			
			byte* bytes;
			
			node.serialize< Contents >( &bytes );
			auto nodePtr = ( Node* ) OctreeNode::deserialize< Contents >( bytes );
			Contents resultContents = nodePtr->getContents();
			
			ASSERT_EQ( resultContents, contents );
			Serializer::dispose( bytes );
			delete nodePtr;
		}
		
		TEST_F( SerializerTest, PointNode )
		{
			SPV_BitMapMemoryManager::initInstance( 1000000 );
			
			Point p0( vec3( 11.321565f, 4.658535f, 7.163479f ), vec3( 7.163479f, 4.658535f, 11.321565f ) );
			Point p1( vec3( 11.201763f, 5.635769f, 6.996898f ), vec3( 6.996898f, 5.635769f, 11.201763f ) );
			Point p2( vec3( 11.198129f, 4.750132f, 7.202037f ), vec3( 7.202037f, 4.750132f, 11.198129f ) );
			
			PointPtr pointArray[3] = { PointPtr( new Point( p0 ) ), PointPtr( new Point( p1 ) ), PointPtr( new Point( p2 ) ) };
			PointVector points( pointArray, pointArray + 3 );
			
			auto node = new LeafNode< PointVector >();
			node->setContents( points );
			
			byte* bytes;
			auto genericNode = ( OctreeNode* ) node;
			genericNode->serialize< PointVector >( &bytes );
			
			OctreeNode* deserializedNode = OctreeNode::deserialize< PointVector >( bytes );
			PointVector deserializedPoints = deserializedNode->getContents< PointVector >();
			
			float epsilon = 1.e-15;
			for( int i = 0; i < points.size(); ++i )
			{
				ASSERT_TRUE( points[ i ]->equal( *deserializedPoints[ i ], epsilon ) );
			}
			
			Serializer::dispose( bytes );
			delete node;
			cout << "Before node deletion" << endl;
			delete deserializedNode;
		}
		
		TEST_F( SerializerTest, ExtendedPointNode )
		{
			using Point = ExtendedPoint;
			using PointPtr = ExtendedPointPtr;
			using PointVector = ExtendedPointVector;
			
			SEV_BitMapMemoryManager::initInstance( 1000000 );
			
			PointPtr p0( new Point( vec3( 0.01f, 0.02f, 0.03f ), vec3( 0.01f, 0.02f, 0.03f ), vec3( 1.f, 15.f ,2.f ) ) );
			PointPtr p1( new Point( vec3( 0.04f, 0.05f, 0.06f ), vec3( 0.04f, 0.05f, 0.06f ), vec3( 3.f, -31.f ,4.f ) ) );
			PointPtr p2( new Point( vec3( 0.07f, 0.08f, 0.09f ), vec3( 0.07f, 0.08f, 0.09f ), vec3( -14.f, 5.f ,6.f ) ) );
			PointPtr pointArray[3] = { p0, p1, p2 };
			PointVector points( pointArray, pointArray + 3 );
			
			auto node = new InnerNode< PointVector >();
			node->setContents( points );
			
			byte* bytes;
			auto genericNode = ( OctreeNode* ) node;
			genericNode->serialize< PointVector >( &bytes );
			
			OctreeNode* deserializedNode = OctreeNode::deserialize< PointVector >( bytes );
			PointVector deserializedPoints = deserializedNode->getContents< PointVector >();
			
			float epsilon = 1.e-15;
			for( int i = 0; i < points.size(); ++i )
			{
				ASSERT_TRUE( points[ i ]->equal( *deserializedPoints[ i ], epsilon ) );
			}
			
			Serializer::dispose( bytes );
			delete node;
			delete deserializedNode;
		}
	}
}