#include <gtest/gtest.h>
#include <iostream>
#include "HeapPointReader.h"
#include <Profiler.h>
#include "MortonCode.h"

using namespace std;
using namespace model;

namespace util
{
	namespace test
	{
        class HeapPointReaderTest : public ::testing::Test
		{
			void SetUp()
			{
				setlocale( LC_NUMERIC, "C" );
			}
		};

		TEST_F( HeapPointReaderTest, Read )
		{
			using M = MediumMortonCode;
			using Dim = OctreeDimensions< M >;
			
			M previousCode; previousCode.build( 0x1 );
			
			HeapPointReader< M > reader( "/media/vinicius/data/Datasets/StMathew/StMathewWithFaces.ply", 7 );
			
			Dim dim = reader.dimensions();
			
			cout << "Reader dim: " << endl << dim << endl << endl;
			
			reader.read(
				[ & ]( const Point& p )
				{
					M currentCode = dim.calcMorton( p );
					ASSERT_LE( previousCode, currentCode );
					previousCode = currentCode;
				}
			);
		}
	}
}