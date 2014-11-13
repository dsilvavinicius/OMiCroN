#include "../model/MortonCode.h"
#include <MortonComparator.h>

#include <gtest/gtest.h>

namespace model
{
	namespace test
	{
        class MortonCodeTest : public ::testing::Test
		{
		protected:
			void SetUp() {}
		};

		TEST_F(MortonCodeTest, Creation)
		{
			ShallowMortonCode shallowMorton;
			
			shallowMorton.build( 7, 5, 0, 3 ); // 10 1100 1011
			ASSERT_EQ( shallowMorton.getBits(), 0x2CBu );
			
			shallowMorton.build(7, 5, 0, 10); // 100 0000 0000 0000 0000 0000 1100 1011;
			ASSERT_EQ( shallowMorton.getBits(), 0x400000CBu );
			
			// Verifying if the unsigned long has 8 bytes.
			ASSERT_EQ( sizeof(unsigned long), 8 );
			
			MediumMortonCode mediumMorton;
			
			mediumMorton.build( 5000, 6000, 7000, 5 ); // 1110 1010 0000 0000
			ASSERT_EQ( mediumMorton.getBits(), 0xEA00ul ); 
			
			mediumMorton.build( 1002999, 501956, 785965, 11 );
			ASSERT_EQ( mediumMorton.getBits(), 0x3616E99CDul );
			
			mediumMorton.build( 5000, 6000, 7000, 13 );
			// 1111 1000 1011 1111 0011 1001 0110 1010 0000 0000
			ASSERT_EQ( mediumMorton.getBits(), 0xF8BF396A00ul );
			
			mediumMorton.build( 5000, 6000, 7000, 21 );
			// 1000 0000 0000 0000 0000 0000 0111 1000 1011 1111 0011 1001 0110 1010 0000 0000
			ASSERT_EQ( mediumMorton.getBits(), 0x80000078BF396A00ul );
		}
		
		TEST_F( MortonCodeTest, Traversal )
		{
			ShallowMortonCode shallowMorton;
			
			shallowMorton.build(7, 5, 0, 3); // 10 1100 1011
			ShallowMortonCodePtr shallowParent = shallowMorton.traverseUp();
			ASSERT_EQ( shallowParent->getBits(), 0x59u );
			
			vector< ShallowMortonCodePtr > shallowChildren = shallowMorton.traverseDown();
			ASSERT_EQ( shallowChildren[ 0 ]->getBits(), 0x1658u );
			ASSERT_EQ( shallowChildren[ 1 ]->getBits(), 0x1659u );
			ASSERT_EQ( shallowChildren[ 2 ]->getBits(), 0x165Au );
			ASSERT_EQ( shallowChildren[ 3 ]->getBits(), 0x165Bu );
			ASSERT_EQ( shallowChildren[ 4 ]->getBits(), 0x165Cu );
			ASSERT_EQ( shallowChildren[ 5 ]->getBits(), 0x165Du );
			ASSERT_EQ( shallowChildren[ 6 ]->getBits(), 0x165Eu );
			ASSERT_EQ( shallowChildren[ 7 ]->getBits(), 0x165Fu );
			
			// Overflow check.
			//shallowMorton.build( 7, 5, 0, 10 ); // 100 0000 0000 0000 0000 0000 1100 1011;
			// TODO: Need to fix the next line.
			//EXPECT_EXIT( shallowMorton.traverseDown(), ::testing::ExitedWithCode( 6 ),
			//			 "shifted > bits && ""MortonCode traversal overflow.""" );
			
			MediumMortonCode mediumMorton;
			
			mediumMorton.build( 1002999, 501956, 785965, 11 ); // 3616E99CD
			MediumMortonCodePtr mediumParent = mediumMorton.traverseUp();
			ASSERT_EQ( mediumParent->getBits(), 0x6C2DD339ul );
			
			vector< MediumMortonCodePtr > mediumChildren = mediumMorton.traverseDown();
			ASSERT_EQ( mediumChildren[ 0 ]->getBits(), 0x1B0B74CE68ul );
			ASSERT_EQ( mediumChildren[ 1 ]->getBits(), 0x1B0B74CE69ul );
			ASSERT_EQ( mediumChildren[ 2 ]->getBits(), 0x1B0B74CE6Aul );
			ASSERT_EQ( mediumChildren[ 3 ]->getBits(), 0x1B0B74CE6Bul );
			ASSERT_EQ( mediumChildren[ 4 ]->getBits(), 0x1B0B74CE6Cul );
			ASSERT_EQ( mediumChildren[ 5 ]->getBits(), 0x1B0B74CE6Dul );
			ASSERT_EQ( mediumChildren[ 6 ]->getBits(), 0x1B0B74CE6Eul );
			ASSERT_EQ( mediumChildren[ 7 ]->getBits(), 0x1B0B74CE6Ful );
			
			// Overflow check. Need to fix.
			//mediumMorton.build(5000, 6000, 7000, 21);
			// TODO: Need to fix the next line.
			//EXPECT_EXIT( mediumMorton.traverseDown(), ::testing::ExitedWithCode( 6 ),
			//			 "shifted > bits && ""MortonCode traversal overflow.""" );
		}
		
		TEST_F(MortonCodeTest, Comparison)
		{
			auto morton0 = make_shared< ShallowMortonCode >();
			morton0->build( 1, 1, 1, 3 );
			
			auto morton1 = make_shared< ShallowMortonCode >();
			morton1->build( 1, 1, 1, 2 );
			
			auto morton2 = make_shared< ShallowMortonCode >();
			morton2->build( 1, 1, 1, 3 );
			
			ASSERT_TRUE(*morton0 != *morton1);
			ASSERT_TRUE(*morton0 == *morton2);
			
			ShallowMortonComparator comp;
			ASSERT_FALSE(comp(morton0, morton1)) << morton0 << " should be greater than " << morton1;
		}
		
		TEST_F(MortonCodeTest, Decoding)
		{	
			// Root node
			unsigned int level = 0;
			ShallowMortonCode shallowMorton;
			shallowMorton.build( 0x1u );
			vector< unsigned int > decoded = shallowMorton.decode( level );
			ASSERT_EQ( decoded[ 0 ], 0 );
			ASSERT_EQ( decoded[ 1 ], 0 );
			ASSERT_EQ( decoded[ 2 ], 0 );
			
			// Leaf (shallow).
			level = 10;
			unsigned int coords[3] = { 7, 5, 0 };
			shallowMorton.build( coords[0], coords[1], coords[2], level );
			decoded = shallowMorton.decode( level );
			
			ASSERT_EQ( decoded[ 0 ], coords[ 0 ] );
			ASSERT_EQ( decoded[ 1 ], coords[ 1 ] );
			ASSERT_EQ( decoded[ 2 ], coords[ 2 ] );
			
			decoded = shallowMorton.decode();
			
			ASSERT_EQ( decoded[ 0 ], coords[ 0 ] );
			ASSERT_EQ( decoded[ 1 ], coords[ 1 ] );
			ASSERT_EQ( decoded[ 2 ], coords[ 2 ] );
			
			// Leaf (medium).
			level = 21;
			unsigned int coordsL[ 3 ] = { 5000, 6000, 7000 };
			MediumMortonCode mediumMorton;
			mediumMorton.build( coordsL[0], coordsL[1], coordsL[2], level );
			vector< unsigned long > decodedL = mediumMorton.decode( level );
			
			ASSERT_EQ( decodedL[ 0 ], coordsL[ 0 ] );
			ASSERT_EQ( decodedL[ 1 ], coordsL[ 1 ] );
			ASSERT_EQ( decodedL[ 2 ], coordsL[ 2 ] );
			
			decodedL = mediumMorton.decode();
			
			ASSERT_EQ( decodedL[ 0 ], coordsL[ 0 ]);
			ASSERT_EQ( decodedL[ 1 ], coordsL[ 1 ]);
			ASSERT_EQ( decodedL[ 2 ], coordsL[ 2 ]);
		}
	}
}
