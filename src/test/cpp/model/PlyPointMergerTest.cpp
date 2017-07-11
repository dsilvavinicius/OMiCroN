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
			string outputPath = "/media/viniciusdasilva/Expansion Drive/Datasets/David/PlyPointMergerTest/merge_9_10_11.ply";
			
			PlyPointMerger merger( "/media/viniciusdasilva/Expansion Drive/Datasets/David/PlyPointMergerTest/group.gp", outputPath );
			merger.merge();
			
			cout << "Merge generated at " << outputPath << ". Meshlab can be used to visualize the results." << endl << endl;
		}
		
		TEST_F( PlyPointMergerTest, MergeDavid )
		{
			string outputPath = "/media/vinicius/Expansion Drive3/Datasets/David/PlyPointMergerTest/DavidWithFaces.ply";
			
			PlyPointMerger merger( "/media/vinicius/Expansion Drive3/Datasets/David/PlyPointMergerTest/David.gp", outputPath );
			merger.merge();
			
			cout << "Merge generated at " << outputPath << ". Meshlab can be used to visualize the results." << endl << endl;
		}
		
		TEST_F( PlyPointMergerTest, MergeAtlas )
		{
			string outputPath = "/media/viniciusdasilva/Expansion Drive/Datasets/Atlas/PlyPointMergerTest/AtlasWithFaces.ply";
			
			PlyPointMerger merger( "/media/viniciusdasilva/Expansion Drive/Datasets/Atlas/PlyPointMergerTest/Atlas_lab.gp", outputPath );
			merger.merge();
			
			cout << "Merge generated at " << outputPath << ". Meshlab can be used to visualize the results." << endl << endl;
		}
		
		TEST_F( PlyPointMergerTest, MergeStMathew )
		{
			string outputPath = "/media/viniciusdasilva/Expansion Drive/Datasets/StMathew/PlyPointMergerTest/StMathewWithFaces.ply";
			
			PlyPointMerger merger( "/media/viniciusdasilva/Expansion Drive/Datasets/StMathew/PlyPointMergerTest/StMathew_lab.gp", outputPath );
			merger.merge();
			
			cout << "Merge generated at " << outputPath << ". Meshlab can be used to visualize the results." << endl << endl;
		}
	}
}