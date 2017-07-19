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
			void SetUp()
			{
				setlocale( LC_NUMERIC, "C" );
			}
		};

		void test( const string& inputFilename, const string& outputFilename, const int lvl,
				   vector< Point >& out_sortedPoints )
		{
			using M = MediumMortonCode;
			using P = Point;
			using PointSorter = model::PointSorter< M >;
			using OctreeDim = typename PointSorter::OctreeDim;
			
			Json::Value octreeJson;
			
			{
				PointSorter sorter( inputFilename, outputFilename, lvl );
				
				cout << "Sorting." << endl << endl;
				
				auto start = Profiler::now();
				
				octreeJson = sorter.sort();
				
				cout << "Total sorting time (ms): " << Profiler::elapsedTime( start ) << endl << endl;
			}
			
			Vec3 octreeSize( octreeJson[ "size" ][ "x" ].asFloat(), octreeJson[ "size" ][ "y" ].asFloat(),
							 octreeJson[ "size" ][ "z" ].asFloat() );
			Vec3 octreeOrigin( octreeJson[ "origin" ][ "x" ].asFloat(), octreeJson[ "origin" ][ "y" ].asFloat(),
							   octreeJson[ "origin" ][ "z" ].asFloat() );
			
			OctreeDim comp( octreeOrigin, octreeSize, octreeJson[ "depth" ].asUInt() );
			
			// Debug
			{
				cout << "Dim from json: " << comp << endl;
			}
			
			cout << "Validating result." << endl << endl;
			
			PlyPointReader reader( outputFilename );
			out_sortedPoints = vector< Point >( reader.getNumPoints() );
			auto iter = out_sortedPoints.begin();
			
			reader.read(
				[ & ]( const P& p )
				{
					*iter++ = p;
				}
			);
			
			for( int i = 0; i < out_sortedPoints.size() - 1; ++i )
			{
				M morton0 = comp.calcMorton( out_sortedPoints[ i ] );
				M morton1 = comp.calcMorton( out_sortedPoints[ i + 1 ] );
				
				// Debug
// 				{
// 					cout << morton0.getPathToRoot( true ) << ": " << out_sortedPoints[ i ].getPos() << endl;
// 				}
				
				ASSERT_TRUE( morton0 < morton1 || morton0 == morton1 );
			}
		}
		
		TEST_F( PointSorterTest, Sort )
		{
			vector< Point > expectedPoints( 11 );
			
			expectedPoints[ 0 ] = Point( Vec3( 0.13f, 0.14f, 0.15f ), Vec3( 0.306667f, 0.546667f, 1.f ) );
			expectedPoints[ 1 ] = Point( Vec3( 0.16f, 0.17f, 0.18f ), Vec3( 0.333333f, 0.573333f, 0.f ) );
			expectedPoints[ 2 ] = Point( Vec3( 0.01f, 0.02f, 0.03f ), Vec3( 0.2f, 0.613333f, 0.706667f ) );
			expectedPoints[ 3 ] = Point( Vec3( 0.04f, 0.05f, 0.06f ), Vec3( 0.226667f, 0.f, 0.733333f ) );
			expectedPoints[ 4 ] = Point( Vec3( 0.19f, 0.2f, 0.21f ), Vec3( 0.36f, 0.253333f, 0.8f ) );
			expectedPoints[ 5 ] = Point( Vec3( 0.22f, 0.23f, 0.24f ), Vec3( 0.12f, 0.306667f, 0.693333f ) );
			expectedPoints[ 6 ] = Point( Vec3( 0.28f, 0.29f, 0.3f ), Vec3( 0.28f, 0.453333f, 0.52f ) );
			expectedPoints[ 7 ] = Point( Vec3( 0.31f, 0.32f, 0.33f ), Vec3( 0.346667f, 0.48f, 0.68f ) );
			expectedPoints[ 8 ] = Point( Vec3( 0.25f, 0.26f, 0.27f ), Vec3( 0.373333f, 0.56f, 0.626667f ) );
			expectedPoints[ 9 ] = Point( Vec3( 0.07f, 0.08f, 0.09f ), Vec3( 0.f, 0.48f, 0.76f ) );
			expectedPoints[ 10 ] = Point( Vec3( 0.1f, 0.11f, 0.12f ), Vec3( 0.8f, 0.506667f, 0.786667f ) );

			vector< Point > sortedPoints;
			test( "data/simple_point_octree.ply", "data/sorted_simple_point_octree.ply", 10, sortedPoints );
			
			ASSERT_EQ( expectedPoints.size(), sortedPoints.size() );
			
			for( int i = 0; i < expectedPoints.size(); ++i )
			{
				ASSERT_TRUE( expectedPoints[ i ].equal( sortedPoints[ i ], 1.e-6 ) );
			}
		}
		
		TEST_F( PointSorterTest, DISABLED_Stress )
		{
			vector< Point > sortedPoints;
			test( "../../../src/data/real/tempietto_sub_tot.ply", "../../../src/data/real/sorted_tempietto_sub_tot.ply",
				  20, sortedPoints );
		}
		
		TEST_F( PointSorterTest, David )
		{
			using M = ShallowMortonCode;
			using PointSorter = model::PointSorter< M >;
			
			int sortLevel = 9;
			
			PointSorter sorter( "/media/viniciusdasilva/Expansion Drive/Datasets/David/PlyPointMergerTest/DavidWithFaces.ply",
								"/media/viniciusdasilva/Expansion Drive/Datasets/David/PlyPointMergerTest/DavidWithFaces_sorted9.ply", sortLevel );
			sorter.sort();
		}
		
		TEST_F( PointSorterTest, Atlas )
		{
			using M = ShallowMortonCode;
			using PointSorter = model::PointSorter< M >;
			
			int sortLevel = 9;
			
			PointSorter sorter( "/media/viniciusdasilva/Expansion Drive/Datasets/Atlas/PlyPointMergerTest/AtlasWithFaces.ply",
								"/media/viniciusdasilva/Expansion Drive/Datasets/Atlas/PlyPointMergerTest/AtlasWithFaces_sorted9.ply", sortLevel );
			sorter.sort();
		}
		
		TEST_F( PointSorterTest, StMathew )
		{
			using M = ShallowMortonCode;
			using PointSorter = model::PointSorter< M >;
			
			int sortLevel = 9;
			
			PointSorter sorter( "/media/viniciusdasilva/Expansion Drive/Datasets/StMathew/PlyPointMergerTest/StMathewWithFaces.ply",
								"/media/viniciusdasilva/Expansion Drive/Datasets/StMathew/PlyPointMergerTest/StMathewWithFaces_sorted9.ply", sortLevel );
			sorter.sort();
		}
	}
}