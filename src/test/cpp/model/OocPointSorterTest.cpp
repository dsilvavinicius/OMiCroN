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
		
		template< typename P >
		void pushToExpectedVector( vector< P >& expected, const P& p )
		{
			for( int i = 0; i < 3; ++i )
			{
				expected.push_back( p );
			}
		}
		
		TEST_F( OocPointSorterTest, Sort )
		{
			using P = Point;
			using M = ShallowMortonCode;
			using Sorter = OocPointSorter< ShallowMortonCode, Point >;
			using Reader = PlyPointReader< P >;
			using OctreeDim = typename Sorter::OctreeDim;
			
			Sorter sorter( "/home/vinicius/Projects/PointBasedGraphics/Cumulus/src/test/data/OocPointSorterTest.gp",
						   "/home/vinicius/Projects/PointBasedGraphics/Cumulus/src/test/data", 10, 1554, 512 );
			
			sorter.sort();
			
			const OctreeDim& comp = sorter.comp();
			
			vector< P > expectedPoints;
			
			pushToExpectedVector( expectedPoints, P( Vec3( 0.13f, 0.14f, 0.15f ), Vec3( 0.306667f, 0.546667f, 1.f ) ) );
			pushToExpectedVector( expectedPoints, P( Vec3( 0.16f, 0.17f, 0.18f ), Vec3( 0.333333f, 0.573333f, 0.f ) ) );
			pushToExpectedVector( expectedPoints, P( Vec3( 0.01f, 0.02f, 0.03f ), Vec3( 0.2f, 0.613333f, 0.706667f ) ) );
			pushToExpectedVector( expectedPoints, P( Vec3( 0.04f, 0.05f, 0.06f ), Vec3( 0.226667f, 0.f, 0.733333f ) ) );
			pushToExpectedVector( expectedPoints, P( Vec3( 0.19f, 0.2f, 0.21f ), Vec3( 0.36f, 0.253333f, 0.8f ) ) );
			pushToExpectedVector( expectedPoints, P( Vec3( 0.22f, 0.23f, 0.24f ), Vec3( 0.12f, 0.306667f, 0.693333f ) ) );
			pushToExpectedVector( expectedPoints, P( Vec3( 0.28f, 0.29f, 0.3f ), Vec3( 0.28f, 0.453333f, 0.52f ) ) );
			pushToExpectedVector( expectedPoints, P( Vec3( 0.31f, 0.32f, 0.33f ), Vec3( 0.346667f, 0.48f, 0.68f ) ) );
			pushToExpectedVector( expectedPoints, P( Vec3( 0.25f, 0.26f, 0.27f ), Vec3( 0.373333f, 0.56f, 0.626667f ) ) );
			pushToExpectedVector( expectedPoints, P( Vec3( 0.07f, 0.08f, 0.09f ), Vec3( 0.f, 0.48f, 0.76f ) ) );
			pushToExpectedVector( expectedPoints, P( Vec3( 0.1f, 0.11f, 0.12f ), Vec3( 0.8f, 0.506667f, 0.786667f ) ) );
			
			ASSERT_EQ( 33, expectedPoints.size() );
			
			Reader reader( "/home/vinicius/Projects/PointBasedGraphics/Cumulus/src/test/data/sorted.ply" );
			vector< P > sortedPoints( reader.getNumPoints() );
			auto iter = sortedPoints.begin();
			
			reader.read(
				[ & ]( const P& p )
				{
					*iter++ = p;
				}
			);
			
			ASSERT_EQ( 33, sortedPoints.size() );
			
			for( int i = 0; i < sortedPoints.size() - 1; ++i )
			{
				M morton0 = comp.calcMorton( sortedPoints[ i ] );
				M morton1 = comp.calcMorton( sortedPoints[ i + 1 ] );
				
				// Debug
				{
					cout << morton0.getPathToRoot( true ) << ": " << sortedPoints[ i ].getPos() << endl << "Expected: "
						 << expectedPoints[ i ].getPos() << endl << endl;
				}
				
				ASSERT_TRUE( morton0 < morton1 || morton0 == morton1 );
				ASSERT_TRUE( expectedPoints[ i ].equal( sortedPoints[ i ], 1.e-6 ) );
			}
			
			ASSERT_TRUE( expectedPoints[ expectedPoints.size() - 1 ].equal( sortedPoints[ sortedPoints.size() - 1 ], 1.e-6 ) );
		}
	}
}