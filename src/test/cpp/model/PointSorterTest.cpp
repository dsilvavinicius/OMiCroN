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
			using P = ExtendedPoint;
			
			PointSorter< M, P > sorter( "data/extended_point_octree.ply", 20 );
			sorter.sort( "data/sorted_extended_point_octree.ply" );
			
			vector< P > sortedPoints;
			PlyPointReader< P > reader( "data/sorted_extended_point_octree.ply" );
			reader.read( PlyPointReader< P >::SINGLE,
				[ & ]( const P& p )
				{
					sortedPoints.push_back( p );
				}
			);
			
			cout << "Sorted points: " << endl << sortedPoints << endl << endl;
		}
	}
}