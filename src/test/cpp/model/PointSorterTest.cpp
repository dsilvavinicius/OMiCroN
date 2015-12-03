#include <gtest/gtest.h>
#include <iostream>
#include "PointSorter.h"
#include "Stream.h"
#include "Profiler.h"

using namespace std;
using namespace util;

namespace model
{
	namespace test
	{
        class PointSorterTest : public ::testing::Test
		{
		protected:
			void SetUp() {}
		};

		TEST_F( PointSorterTest, Sort )
		{
			using M = MediumMortonCode;
			using P = Point;
			using PointSorter = model::PointSorter< M, P >;
			
			auto start = Profiler::now();
			
			PointSorter sorter( "../../../src/data/real/tempietto_sub_tot.ply", 20 );
			
			cout << "Sorting." << endl << endl;
			
			sorter.sort( "../../../src/data/real/sorted_tempietto_sub_tot.ply" );
			
			cout << "Total sorting time (ms): " << Profiler::elapsedTime( start ) << endl << endl;
			
			cout << "Validating result." << endl << endl;
			
			vector< P > sortedPoints;
			PlyPointReader< P > reader( "../../../src/data/real/sorted_tempietto_sub_tot.ply" );
			reader.read( PlyPointReader< P >::SINGLE,
				[ & ]( const P& p )
				{
					sortedPoints.push_back( p );
				}
			);
			
			typename PointSorter::OctreeDim comp = sorter.comp();
			
			for( int i = 0; i < sortedPoints.size() - 1; ++i )
			{
				M morton0 = comp.calcMorton( sortedPoints[ i ] );
				M morton1 = comp.calcMorton( sortedPoints[ i + 1 ] );
				ASSERT_TRUE( morton0 < morton1 || morton0 == morton1 );
			}
		}
	}
}