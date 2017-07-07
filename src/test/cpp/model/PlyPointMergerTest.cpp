#include "PlyPointMerger.h"

#include <gtest/gtest.h>
#include <iostream>

using namespace std;

namespace model
{
	namespace test
	{
        class PlyPointMergerTest : public ::testing::Test
		{
		protected:
			void SetUp() {}
		};

		TEST_F( PlyPointMergerTest, Merge )
		{
			string outputPath = "/media/vinicius/Expansion Drive3/Datasets/David/PlyPointMergerTest/merge.ply";
			
			PlyPointMerger merger( "/media/vinicius/Expansion Drive3/Datasets/David/PlyPointMergerTest/group.gp", outputPath );
			merger.merge();
			
			cout << "Merge generated at " << outputPath << ". Meshlab can be used to visualize the results." << endl << endl;
		}
	}
}