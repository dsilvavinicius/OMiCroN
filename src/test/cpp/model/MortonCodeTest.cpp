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
			unsigned int ibits = shallowMorton.getBits();
			ASSERT_EQ(ibits, 0x2CB) << "Expected 0x2CB, received " << hex << ibits; // 10 1100 1011
			
			shallowMorton.build(7, 5, 0, 10); // 100 0000 0000 0000 0000 0000 1100 1011
			ibits = shallowMorton.getBits();
			ASSERT_EQ(ibits, 0x400000CB) << "Expected 0x400000CB, received " << hex << ibits;
			
			// Verifying if the unsigned long has 8 bytes.
			ASSERT_EQ(sizeof(unsigned long), 8);
			
			MediumMortonCode mediumMorton;
			
			mediumMorton.build(5000, 6000, 7000, 5);
			unsigned long lbits = mediumMorton.getBits();
			ASSERT_EQ(lbits, 0xEA00) << "Expected 0xEA00, received " << hex << lbits; // 1110 1010 0000 0000
			
			mediumMorton.build(5000, 6000, 7000, 13);
			lbits = mediumMorton.getBits();
			// 1111 1000 1011 1111 0011 1001 0110 1010 0000 0000
			ASSERT_EQ(lbits, 0xF8BF396A00) << "Expected 0xF8BF396A00, received " << hex << lbits;
			
			mediumMorton.build(5000, 6000, 7000, 21);
			lbits = mediumMorton.getBits();
			// 1000 0000 0000 0000 0000 0000 0111 1000 1011 1111 0011 1001 0110 1010 0000 0000
			ASSERT_EQ(lbits, 0x80000078BF396A00)  << "Expected 0x80000078BF396A00, received " << hex << lbits;
		}
		
		TEST_F(MortonCodeTest, Traversal) {
			ShallowMortonCode shallowMorton;
			
			shallowMorton.build(7, 5, 0, 3);
			shallowMorton.traverseUp(shallowMorton);
		}
	}
}
