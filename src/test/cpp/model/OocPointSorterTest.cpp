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
			void SetUp()
			{
				setlocale( LC_NUMERIC, "C" );
			}
		};
		
		template< typename P >
		void pushToExpectedVector( vector< P >& expected, const P& p )
		{
			for( int i = 0; i < 3; ++i )
			{
				expected.push_back( p );
			}
		}
		
		template< typename M >
		void test( OocPointSorter< M >& sorter, const string& octreeFilename, ulong expectedNumPoints )
		{
			using Reader = PlyPointReader;
			using OctreeDim = typename OocPointSorter< M >::OctreeDim;
			
			auto start = Profiler::now( "Sorting David, 13 lvls," );
			
			sorter.sort( true );
			
			Profiler::elapsedTime( start, "Sorting David, 13 lvls," );
			
			ifstream ifs( octreeFilename );
			Json::Value octreeJson;
			ifs >> octreeJson;
			
			Vec3 octreeSize( octreeJson[ "size" ][ "x" ].asFloat(), octreeJson[ "size" ][ "y" ].asFloat(),
							 octreeJson[ "size" ][ "z" ].asFloat() );
			Vec3 octreeOrigin( octreeJson[ "origin" ][ "x" ].asFloat(), octreeJson[ "origin" ][ "y" ].asFloat(),
							 octreeJson[ "origin" ][ "z" ].asFloat() );
			
			OctreeDim comp( octreeOrigin, octreeSize, octreeJson[ "depth" ].asUInt() );
			
			// Debug
			{
				cout << "Octree dim: " << comp << endl;
			}
			
			Reader reader( octreeJson[ "points" ].asString() );
			
			ASSERT_EQ( expectedNumPoints, reader.getNumPoints() );
			
			Point prev;
			
			bool init = false;
			
			reader.read(
				[ & ]( const Point& p )
				{
					if( init )
					{
						M morton0 = comp.calcMorton( prev );
						M morton1 = comp.calcMorton( p );
						
						ASSERT_TRUE( morton0 < morton1 || morton0 == morton1 );
						
						prev = p;
					}
					else
					{
						M morton = comp.calcMorton( p );
						
						prev = p;
						init = true;
					}
				}
			);
		}
		
		void test( const string& groupFilename, const string& outputFolder, int lvls, ulong totalSizeInBytes,
				   ulong memoryQuota, ulong expectedNumPoints )
		{
			OocPointSorter< MediumMortonCode > sorter( groupFilename, outputFolder, lvls, totalSizeInBytes,
													   memoryQuota );
			
			int nameBeginIdx = ( groupFilename.find_last_of( '/' ) == groupFilename.npos ) ? 0
							: groupFilename.find_last_of( '/' ) + 1;
			int nameEndIdx = groupFilename.find_last_of( '.' );
			string datasetName = groupFilename.substr( nameBeginIdx, nameEndIdx - nameBeginIdx );
			
			test( sorter, outputFolder + "/" + datasetName + ".oct", expectedNumPoints );
		}
		
		TEST_F( OocPointSorterTest, Sort )
		{
			using P = Point;
			using Reader = PlyPointReader;
			
			test( "data/OocPointSorterTest.gp", "data", 10, 1554, 512, 33 );
			
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
			
			vector< P > sortedPoints;
			Reader reader( "data/OocPointSorterTest.ply" );
			
			reader.read(
				[ & ]( const Point& p )
				{
					sortedPoints.push_back( p );
				}
			);
			
			ASSERT_EQ( expectedPoints.size(), sortedPoints.size() );
			
			for( int i = 0; i < expectedPoints.size(); ++i )
			{
				ASSERT_TRUE( expectedPoints[ i ].equal( sortedPoints[ i ], 1.e-6 ) );
			}
		}
		
		TEST_F( OocPointSorterTest, HeavierDataset )
		{
			test( "/media/vinicius/Expansion Drive3/Datasets/David/test/test.gp",
				  "/media/vinicius/Expansion Drive3/Datasets/David/test", 10, 73ul * 1024ul * 1024ul, 20ul * 1024 * 1024,
				  1325568ul );
		}
		
		TEST_F( OocPointSorterTest, DISABLED_David )
		{
			test( "/media/vinicius/Expansion Drive3/Datasets/David/PlyFilesFlippedNormals/David.gp",
				  "/media/vinicius/Expansion Drive3/Datasets/David/Sorted_11Lvls", 11,
				  ulong( 25.8 * 1024ul * 1024ul * 1024ul ), 10ul * 1024ul * 1024ul * 1024ul, 468640353ul );
		}
		
		TEST_F( OocPointSorterTest, StMathew )
		{
			test( "/media/vinicius/Expansion Drive3/Datasets/StMathew/StMathew.gp",
				  "/media/vinicius/Expansion Drive3/Datasets/StMathew/Sorted_21Lvls", 21,
				  ulong( 10.1 * 1024ul * 1024ul * 1024ul ), ulong( 10.1 * 1024ul * 1024ul * 1024ul ), 186984410ul );
		}
		
		TEST_F( OocPointSorterTest, DISABLED_DavidResort )
		{
			using Sorter = OocPointSorter< MediumMortonCode >;
			using OctreeDim = typename Sorter::OctreeDim;
			
			string plyGroupFilename = "/media/vinicius/Expansion Drive3/Datasets/David/Sorted_13Lvls/David.gp";
			string octreeFilename = "/media/vinicius/Expansion Drive3/Datasets/David/Sorted_11Lvls/David.oct";
			string outputFolder = "/media/vinicius/Expansion Drive3/Datasets/David/Sorted_13Lvls";
			ulong totalSize = ulong( 25.8 * 1024ul * 1024ul * 1024ul );
			ulong memoryQuota = 10ul * 1024ul * 1024ul * 1024ul;
			
			ifstream ifs( octreeFilename );
			Json::Value octreeJson;
			ifs >> octreeJson;
			Vec3 octreeSize( octreeJson[ "size" ][ "x" ].asFloat(), octreeJson[ "size" ][ "y" ].asFloat(),
							 octreeJson[ "size" ][ "z" ].asFloat() );
			Vec3 octreeOrigin( octreeJson[ "origin" ][ "x" ].asFloat(), octreeJson[ "origin" ][ "y" ].asFloat(),
							   octreeJson[ "origin" ][ "z" ].asFloat() );
			
			OctreeDim dim( octreeOrigin, octreeSize, 11 );
			
			Sorter sorter( plyGroupFilename, outputFolder, dim, totalSize, memoryQuota );
			test( sorter, octreeFilename, 468640353ul );
		}
	}
}