#include <gtest/gtest.h>
#include <iostream>
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
			
			Sorter sorter( "/media/vinicius/Expansion Drive3/Datasets/David/PlyFiles/test.gp",
						   "/media/vinicius/Expansion Drive3/Datasets/test", 10, 57 * 1024 * 1024, 25 * 1024 * 1024 );
			
			sorter.sort();
		}
	}
}