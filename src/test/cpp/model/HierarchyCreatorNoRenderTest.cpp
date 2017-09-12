#include "HierarchyCreator.h"
#include "OctreeFile.h"

#include <gtest/gtest.h>
#include <iostream>
#include <jsoncpp/json/json.h>

#ifndef HIERARCHY_CREATION_RENDERING
	using namespace std;

	namespace model
	{
		namespace test
		{
			class HierarchyCreatorNoRenderTest : public ::testing::Test
			{
			protected:
				void SetUp() {
					setlocale( LC_NUMERIC, "C" );
				}
			};

			void test( const string& inputFile, const string& outputFile )
			{
				using Morton = MediumMortonCode;
				using Dim = OctreeDimensions< Morton >;
				using Creator = HierarchyCreator< Morton >;
				using Node = typename Creator::Node;
				
				ifstream file( inputFile );
				Json::Value octreeJson;
				file >> octreeJson;
				
				cout << "Octree json: " << octreeJson << endl << endl;
				
				Vec3 octreeSize( octreeJson[ "size" ][ "x" ].asFloat(),
						 octreeJson[ "size" ][ "y" ].asFloat(),
						 octreeJson[ "size" ][ "z" ].asFloat() );
				Vec3 octreeOrigin( octreeJson[ "origin" ][ "x" ].asFloat(),
						   octreeJson[ "origin" ][ "y" ].asFloat(),
						   octreeJson[ "origin" ][ "z" ].asFloat() );
		
				Dim dim( octreeOrigin, octreeSize, octreeJson[ "depth" ].asUInt() );
				
				cout << "Octree dim: " << dim << endl << endl;
				
				Creator creator( octreeJson[ "points" ].asString(), dim, WORK_LIST_SIZE, RAM_QUOTA, HIERARCHY_CREATION_THREADS );
				pair< Node*, int > result = creator.createAsync().get();
				
				unique_ptr< Node > root( result.first );
				
				auto now = Profiler::now( "Save octree operation" );
				
				OctreeFile::write( outputFile, *root );
				
				Profiler::elapsedTime( now, "Save octree operation" );
			}
			
			TEST_F( HierarchyCreatorNoRenderTest, David)
			{
				test( "/media/vinicius/data/Datasets/David/DavidWithFaces_sorted7.oct", "/media/vinicius/data/Datasets/David/David.boc" );
			}
			
			TEST_F( HierarchyCreatorNoRenderTest, Atlas )
			{
				test( "/media/vinicius/data/Datasets/Atlas/AtlasWithFaces_sorted7.oct", "/media/vinicius/data/Datasets/Atlas/Atlas.boc" );
			}
			
			TEST_F( HierarchyCreatorNoRenderTest, StMathew )
			{
				test( "/media/vinicius/data/Datasets/StMathew/StMathewWithFaces_sorted7.oct", "/media/vinicius/data/Datasets/StMathew/StMathew.boc" );
			}
		}
	}
#endif