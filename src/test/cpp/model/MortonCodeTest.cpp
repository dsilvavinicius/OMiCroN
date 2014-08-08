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
		
		TEST_F(MortonCodeTest, Traversal)
		{
			ShallowMortonCode shallowMorton;
			
			shallowMorton.build(7, 5, 0, 3); // 10 1100 1011
			ShallowMortonCodePtr parent = shallowMorton.traverseUp();
			ASSERT_EQ(parent->getBits(), 0x59);
			
			vector< ShallowMortonCodePtr > children = shallowMorton.traverseDown();
			ASSERT_EQ(children[0]->getBits(), 0x1658);
			ASSERT_EQ(children[1]->getBits(), 0x1659);
			ASSERT_EQ(children[2]->getBits(), 0x165A);
			ASSERT_EQ(children[3]->getBits(), 0x165B);
			ASSERT_EQ(children[4]->getBits(), 0x165C);
			ASSERT_EQ(children[5]->getBits(), 0x165D);
			ASSERT_EQ(children[6]->getBits(), 0x165E);
			ASSERT_EQ(children[7]->getBits(), 0x165F);
			
			// Checks for overflow.
			shallowMorton.build(7, 5, 0, 10); // 100 0000 0000 0000 0000 0000 1100 1011;
			ASSERT_ANY_THROW(shallowMorton.traverseDown());
			
			MediumMortonCode mediumMorton;
			mediumMorton.build(5000, 6000, 7000, 21);
			ASSERT_ANY_THROW(mediumMorton.traverseDown());
		}
		
		TEST_F(MortonCodeTest, Comparison)
		{
			auto morton0 = make_shared< ShallowMortonCode >();
			morton0->build(1, 1, 1, 3);
			
			auto morton1 = make_shared< ShallowMortonCode >();
			morton1->build(1, 1, 1, 2);
			
			auto morton2 = make_shared< ShallowMortonCode >();
			morton2->build(1, 1, 1, 3);
			
			ASSERT_TRUE(*morton0 != *morton1);
			ASSERT_TRUE(*morton0 == *morton2);
			
			ShallowMortonComparator comp;
			ASSERT_FALSE(comp(morton0, morton1)) << morton0 << " should be greater than " << morton1;
		}
		
		TEST_F(MortonCodeTest, Decoding)
		{
			unsigned int level = 10;
			unsigned int coords[3] = { 7, 5, 0 };
			ShallowMortonCode shallowMorton;
			shallowMorton.build(coords[0], coords[1], coords[2], level);
			vector< unsigned int > decoded = shallowMorton.decode(level);
			
			ASSERT_EQ(decoded[0], coords[0]);
			ASSERT_EQ(decoded[1], coords[1]);
			ASSERT_EQ(decoded[2], coords[2]);
			
			level = 21;
			unsigned int coordsL[3] = { 5000, 6000, 7000 };
			MediumMortonCode mediumMorton;
			mediumMorton.build(coordsL[0], coordsL[1], coordsL[2], level);
			vector< unsigned long > decodedL = mediumMorton.decode(level);
			
			ASSERT_EQ(decodedL[0], coordsL[0]);
			ASSERT_EQ(decodedL[1], coordsL[1]);
			ASSERT_EQ(decodedL[2], coordsL[2]);
		}
	}
}
