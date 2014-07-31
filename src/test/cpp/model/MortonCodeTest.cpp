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
			
			shallowMorton.build(7, 5, 0, 3);
			ASSERT_EQ(shallowMorton.getBits(), 0x2CB); // 10 1100 1011
			
			shallowMorton.build(7, 5, 0, 10); // 100 0000 0000 0000 0000 0000 1100 1011
			ASSERT_EQ(shallowMorton.getBits(), 0x400000CB);
			
			MediumMortonCode mediumMorton;
			
			mediumMorton.build(5000, 6000, 7000, 5);
			ASSERT_EQ(mediumMorton.getBits(), 0xDA00); // 1110 1010 0000 0000
			
			mediumMorton.build(5000, 6000, 7000, 13);
			ASSERT_EQ(mediumMorton.getBits(), 0x78BF396A00); // 0111 1000 1011 1111 0011 1001 0110 1010 0000 0000
			
			mediumMorton.build(5000, 6000, 7000, 21);
			// 1000 0000 0000 0000 0000 0000 0111 1000 1011 1111 0011 1001 0110 1010 0000 0000
			ASSERT_EQ(mediumMorton.getBits(), 0x80000078BF396A00);
		}
		
		TEST_F(MortonCodeTest, Traversal) {
			ShallowMortonCode shallowMorton;
			
			shallowMorton.build(7, 5, 0, 3);
			shallowMorton.traverseUp(shallowMorton);
		}
	}
}
