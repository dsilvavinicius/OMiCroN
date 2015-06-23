#include <gtest/gtest.h>
#include "Serializer.h"
#include <OctreeNode.h>

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
			Point p0( vec3( 11.321565f, 4.658535f, 7.163479f ), vec3( 7.163479f, 4.658535f, 11.321565f ) );
			Point p1( vec3( 11.201763f, 5.635769f, 6.996898f ), vec3( 6.996898f, 5.635769f, 11.201763f ) );
			Point p2( vec3( 11.198129f, 4.750132f, 7.202037f ), vec3( 7.202037f, 4.750132f, 11.198129f ) );
			
			PointPtr pointArray[3] = { make_shared< Point >( p0 ), make_shared< Point >( p1 ), make_shared< Point >( p2 ) };
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
		}
		
		TEST_F( SerializerTest, ExtendedPointVector )
		{
			using Point = ExtendedPoint;
			using PointPtr = ExtendedPointPtr;
			using PointVector = ExtendedPointVector;
			
			auto p0 = make_shared< Point >( vec3( 0.01f, 0.02f, 0.03f ), vec3( 0.01f, 0.02f, 0.03f ),
											vec3( 1.f, 15.f ,2.f ) );
			auto p1 = make_shared< Point >( vec3( 0.04f, 0.05f, 0.06f ), vec3( 0.04f, 0.05f, 0.06f ),
											vec3( 3.f, -31.f ,4.f ) );
			auto p2 = make_shared< Point >( vec3( 0.07f, 0.08f, 0.09f ), vec3( 0.07f, 0.08f, 0.09f ),
											vec3( -14.f, 5.f ,6.f ) );
			
			PointPtr pointArray[3] = { p0, p1, p2 };
			PointVector points( pointArray, pointArray + 3 );
			
			byte* bytes;
			Serializer::serialize( points, &bytes );
			
			PointVector deserializedPoints;
			Serializer::deserialize( bytes, deserializedPoints );
			
			float epsilon = 1.e-15;
			for( int i = 0; i < points.size(); ++i )
			{
				cout << "Deserialized point: " << *deserializedPoints[ i ] << "Expected: " << *points[ i ] << endl;
				ASSERT_TRUE( points[ i ]->equal( *deserializedPoints[ i ], epsilon ) );
			}
		}
		
		TEST_F( SerializerTest, LeafIndexNode )
		{
			using Contents = vector< unsigned long >;
			using Node = LeafNode< ShallowMortonCode, Contents >;
			
			Node node;
			unsigned long array[ 3 ] = { 1, 2, 3 };
			Contents contents( array, array + 3 );
			node.setContents( contents );
			
			byte* bytes;
			
			node.serialize< Contents >( &bytes );
			auto nodePtr = ( Node* ) OctreeNode< ShallowMortonCode >::deserialize< Contents >( bytes );
			Contents resultContents = *nodePtr->getContents();
			
			ASSERT_EQ( resultContents, contents );
			delete nodePtr;
		}
		
		TEST_F( SerializerTest, InnerIndexNode )
		{
			using Contents = vector< unsigned long >;
			using Node = InnerNode< ShallowMortonCode, Contents >;
			
			Node node;
			unsigned long array[ 3 ] = { 1, 2, 3 };
			Contents contents( array, array + 3 );
			node.setContents( contents );
			
			byte* bytes;
			
			node.serialize< Contents >( &bytes );
			auto nodePtr = ( Node* ) OctreeNode< ShallowMortonCode >::deserialize< Contents >( bytes );
			Contents resultContents = *nodePtr->getContents();
			
			ASSERT_EQ( resultContents, contents );
			delete nodePtr;
		}
		
		TEST_F( SerializerTest, PointNode )
		{
			Point p0( vec3( 11.321565f, 4.658535f, 7.163479f ), vec3( 7.163479f, 4.658535f, 11.321565f ) );
			Point p1( vec3( 11.201763f, 5.635769f, 6.996898f ), vec3( 6.996898f, 5.635769f, 11.201763f ) );
			Point p2( vec3( 11.198129f, 4.750132f, 7.202037f ), vec3( 7.202037f, 4.750132f, 11.198129f ) );
			
			PointPtr pointArray[3] = { make_shared< Point >( p0 ), make_shared< Point >( p1 ), make_shared< Point >( p2 ) };
			PointVector points( pointArray, pointArray + 3 );
			
			auto node = new ShallowLeafNode< PointVector >();
			node->setContents( points );
			
			byte* bytes;
			auto genericNode = ( ShallowOctreeNode* ) node;
			genericNode->serialize< PointVector >( &bytes );
			
			ShallowOctreeNode* deserializedNode = ShallowOctreeNode::deserialize< PointVector >( bytes );
			PointVector deserializedPoints = *deserializedNode->getContents< PointVector >();
			
			float epsilon = 1.e-15;
			for( int i = 0; i < points.size(); ++i )
			{
				ASSERT_TRUE( points[ i ]->equal( *deserializedPoints[ i ], epsilon ) );
			}
			
			delete node;
			delete deserializedNode;
		}
		
		TEST_F( SerializerTest, ExtendedPointNode )
		{
			using Point = ExtendedPoint;
			using PointPtr = ExtendedPointPtr;
			using PointVector = ExtendedPointVector;
			
			auto p0 = make_shared< Point >( vec3( 0.01f, 0.02f, 0.03f ), vec3( 0.01f, 0.02f, 0.03f ),
											vec3( 1.f, 15.f ,2.f ) );
			auto p1 = make_shared< Point >( vec3( 0.04f, 0.05f, 0.06f ), vec3( 0.04f, 0.05f, 0.06f ),
											vec3( 3.f, -31.f ,4.f ) );
			auto p2 = make_shared< Point >( vec3( 0.07f, 0.08f, 0.09f ), vec3( 0.07f, 0.08f, 0.09f ),
											vec3( -14.f, 5.f ,6.f ) );
			PointPtr pointArray[3] = { p0, p1, p2 };
			PointVector points( pointArray, pointArray + 3 );
			
			auto node = new ShallowInnerNode< PointVector >();
			node->setContents( points );
			
			byte* bytes;
			auto genericNode = ( ShallowOctreeNode* ) node;
			genericNode->serialize< PointVector >( &bytes );
			
			ShallowOctreeNode* deserializedNode = ShallowOctreeNode::deserialize< PointVector >( bytes );
			PointVector deserializedPoints = *deserializedNode->getContents< PointVector >();
			
			float epsilon = 1.e-15;
			for( int i = 0; i < points.size(); ++i )
			{
				ASSERT_TRUE( points[ i ]->equal( *deserializedPoints[ i ], epsilon ) );
			}
			
			delete node;
			delete deserializedNode;
		}
	}
}