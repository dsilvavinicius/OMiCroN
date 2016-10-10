#include "MortonInterval.h"

#include <gtest/gtest.h>
#include <unordered_set>

using namespace std;

namespace model
{
	namespace test
	{
        class MortonIntervalTest : public ::testing::Test
		{
		protected:
			void SetUp() {}
		};

		TEST_F( MortonIntervalTest, EqualityAndHash )
		{
			ShallowMortonCode a; a.build( 0x3 );
			ShallowMortonCode b; a.build( 0x4 );
			ShallowMortonInterval interval0( makeManaged< ShallowMortonCode >( a ), makeManaged< ShallowMortonCode >( b ) );
			ShallowMortonInterval interval1( makeManaged< ShallowMortonCode >( b ), makeManaged< ShallowMortonCode >( a ) );
			ShallowMortonInterval interval2( makeManaged< ShallowMortonCode >( a ), makeManaged< ShallowMortonCode >( b ) );
			ShallowMortonInterval interval3 = interval2;
			
			ASSERT_FALSE( ShallowMortonIntervalComparator()( interval0, interval1 ) );
			ASSERT_TRUE( ShallowMortonIntervalComparator()( interval0, interval2 ) );
			ASSERT_TRUE( ShallowMortonIntervalComparator()( interval2, interval3 ) );
			ASSERT_TRUE( ShallowMortonIntervalComparator()( interval0, interval3 ) );
			
			ASSERT_NE( hash< ShallowMortonInterval >()( interval0 ), hash< ShallowMortonInterval >()( interval1 ) );
			ASSERT_EQ( hash< ShallowMortonInterval >()( interval0 ), hash< ShallowMortonInterval >()( interval2 ) );
			ASSERT_EQ( hash< ShallowMortonInterval >()( interval2 ), hash< ShallowMortonInterval >()( interval3 ) );
			ASSERT_EQ( hash< ShallowMortonInterval >()( interval0 ), hash< ShallowMortonInterval >()( interval3 ) );
		}
		
		TEST_F( MortonIntervalTest, UnorderedSet )
		{
			ShallowMortonCode a; a.build( 0x3 );
			ShallowMortonCode b; a.build( 0x4 );
			
			ShallowMortonInterval interval0( makeManaged< ShallowMortonCode >( a ), makeManaged< ShallowMortonCode >( b ) );
			ShallowMortonInterval interval1( makeManaged< ShallowMortonCode >( b ), makeManaged< ShallowMortonCode >( a ) );
			ShallowMortonInterval interval2( makeManaged< ShallowMortonCode >( a ), makeManaged< ShallowMortonCode >( b ) );
			
			unordered_set< ShallowMortonInterval, hash< ShallowMortonInterval >,
						   MortonIntervalComparator< ShallowMortonInterval > > uSet( { interval0, interval1, interval2 } );
			
			ASSERT_EQ( uSet.size(), 2 );
			ASSERT_NE( uSet.find( interval0 ), uSet.end() );
			ASSERT_NE( uSet.find( interval1 ), uSet.end() );
			ASSERT_NE( uSet.find( interval2 ), uSet.end() );
		}
	}
}