#include <gtest/gtest.h>
#include <iostream>
#include "MortonCode.h"
#include "OocPointSorter.h"

using namespace std;
using namespace util;

namespace model
{
	namespace test
	{
        class OocPointSorterTest : public ::testing::Test
		{
		protected:
			void SetUp() {}
		};
		
		TEST_F( OocPointSorterTest, Sort )
		{
			using Sorter = OocPointSorter< ShallowMortonCode, Point >;
			
			Sorter sorter( "/home/vinicius/Projects/PointBasedGraphics/Cumulus/src/test/data/OocPointSorterTest.gp",
						   "/home/vinicius/Projects/PointBasedGraphics/Cumulus/src/test/data", 10, 1554, 512 );
			
			sorter.sort();
		}
	}
}