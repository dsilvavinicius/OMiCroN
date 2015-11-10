#include <gtest/gtest.h>
#include <iostream>
#include "PointSorter.h"
#include "Stream.h"

using namespace std;

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
			
			clock_t start = clock();
			
			PointSorter sorter( "../../../src/data/real/tempietto_sub_tot.ply", 20 );
			
			cout << "Sorting." << endl << endl;
			
			sorter.sort( "../../../src/data/real/sorted_tempietto_sub_tot.ply" );
			
			cout << "Sorting time: " << float( clock() - start ) / CLOCKS_PER_SEC << "s." << endl << endl;
			
			cout << "Validating result." << endl << endl;
			
			vector< P > sortedPoints;
			PlyPointReader< P > reader( "../../../src/data/real/sorted_tempietto_sub_tot.ply" );
			reader.read( PlyPointReader< P >::SINGLE,
				[ & ]( const P& p )
				{
					sortedPoints.push_back( p );
				}
			);
			
			typename PointSorter::PointComp comp = sorter.comparator();
			
			for( int i = 0; i < sortedPoints.size() - 1; ++i )
			{
				ASSERT_TRUE( comp( sortedPoints[ i ], sortedPoints[ i + 1 ] ) );
			}
		}
	}
}