#include "../model/MortonCode.h"

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

		TEST_F(MortonCodeTest, Creation) {
			ShallowMortonCode shallowMorton;
			
			shallowMorton.build(7, 5, 0, 3); // 10 1100 1011
			ASSERT_EQ(shallowMorton.getBits(), 0x2CB);
			
			shallowMorton.build(7, 5, 0, 10); // 100 0000 0000 0000 0000 0000 1100 1011;
			ASSERT_EQ(shallowMorton.getBits(), 0x400000CB);
			
			// Verifying if the unsigned long has 8 bytes.
			ASSERT_EQ(sizeof(unsigned long), 8);
			
			MediumMortonCode mediumMorton;
			
			mediumMorton.build(5000, 6000, 7000, 5); // 1110 1010 0000 0000
			ASSERT_EQ(mediumMorton.getBits(), 0xEA00); 
			
			mediumMorton.build(5000, 6000, 7000, 13);
			// 1111 1000 1011 1111 0011 1001 0110 1010 0000 0000
			ASSERT_EQ(mediumMorton.getBits(), 0xF8BF396A00);
			
			mediumMorton.build(5000, 6000, 7000, 21);
			// 1000 0000 0000 0000 0000 0000 0111 1000 1011 1111 0011 1001 0110 1010 0000 0000
			ASSERT_EQ(mediumMorton.getBits(), 0x80000078BF396A00);
		}
		
		TEST_F(MortonCodeTest, Traversal) {
			ShallowMortonCode shallowMorton;
			
			shallowMorton.build(7, 5, 0, 3); // 10 1100 1011
			ShallowMortonCode parent = shallowMorton.traverseUp();
			ASSERT_EQ(parent.getBits(), 0x59);
			
			vector< ShallowMortonCode > children = shallowMorton.traverseDown();
			ASSERT_EQ(children[0].getBits(), 0x1658);
			ASSERT_EQ(children[1].getBits(), 0x1659);
			ASSERT_EQ(children[2].getBits(), 0x165A);
			ASSERT_EQ(children[3].getBits(), 0x165B);
			ASSERT_EQ(children[4].getBits(), 0x165C);
			ASSERT_EQ(children[5].getBits(), 0x165D);
			ASSERT_EQ(children[6].getBits(), 0x165E);
			ASSERT_EQ(children[7].getBits(), 0x165F);
			
			// Checks for overflow.
			shallowMorton.build(7, 5, 0, 10); // 100 0000 0000 0000 0000 0000 1100 1011;
			ASSERT_ANY_THROW(shallowMorton.traverseDown());
			
			MediumMortonCode mediumMorton;
			mediumMorton.build(5000, 6000, 7000, 21);
			ASSERT_ANY_THROW(mediumMorton.traverseDown());
		}
	}
}
