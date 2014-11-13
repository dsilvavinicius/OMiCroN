#include "FrontOctree.h"

#include <gtest/gtest.h>

using namespace std;

namespace model
{
	namespace test
	{
        class FrontTest : public ::testing::Test
		{
		protected:
			void SetUp() {}
		};

		TEST_F( FrontTest, Compilation )
		{
			ShallowFront front;
			ShallowMortonCode code;
			code.build( 0x1 );
			
			front.insert( ShallowFront::value_type( 1L, code ) );
			code.build( 0x8 );
			front.insert( ShallowFront::value_type( 10L, code ) );
			code.build( 0x9 );
			front.insert( ShallowFront::value_type( 25L, code ) );
			code.build( 0xA );
			front.insert( ShallowFront::value_type( 100L, code ) );
			code.build( 0xB );
			front.insert( ShallowFront::value_type( 125L, code ) );
		}
	}
}