#include "OctreeNode.h"

#include <gtest/gtest.h>

using namespace std;

namespace model
{
	namespace test
	{
        class OctreeNodeTest : public ::testing::Test
		{
		protected:
			void SetUp() {}
		};
		
		TEST_F( OctreeNodeTest, LeafNodeSerialization )
		{
			using Contents = vector< unsigned long >;
			using Node = LeafNode< unsigned int, float, vec3, Contents >;
			
			Node node;
			unsigned long array[ 3 ] = { 1, 2, 3 };
			Contents contents( array, array + 3 );
			node.setContents( contents );
			
			byte* bytes;
			
			node.serialize< Contents >( &bytes );
			auto nodePtr = ( Node* ) OctreeNode< unsigned int, float, vec3 >::deserialize< Contents >( bytes );
			Contents resultContents = *nodePtr->getContents();
			
			ASSERT_EQ( resultContents, contents );
			delete nodePtr;
		}
		
		TEST_F( OctreeNodeTest, InnerNodeSerialization )
		{
			using Contents = vector< unsigned long >;
			using Node = InnerNode< unsigned int, float, vec3, Contents >;
			
			Node node;
			unsigned long array[ 3 ] = { 1, 2, 3 };
			Contents contents( array, array + 3 );
			node.setContents( contents );
			
			byte* bytes;
			
			node.serialize< Contents >( &bytes );
			auto nodePtr = ( Node* ) OctreeNode< unsigned int, float, vec3 >::deserialize< Contents >( bytes );
			Contents resultContents = *nodePtr->getContents();
			
			ASSERT_EQ( resultContents, contents );
			delete nodePtr;
		}
	}
}