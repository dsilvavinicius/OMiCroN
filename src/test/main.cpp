#include <gtest/gtest.h>
#include <QApplication>
#include <QDir>
#include <string>
#include <boost/program_options.hpp>
#include "hierarchy/fast_parallel_octree_test_param.h"

using namespace std;
namespace po = boost::program_options;

FastParallelOctreeTestParam g_fastParallelStressParam;

int main(int argc, char** argv)
{
	setlocale( LC_NUMERIC, "C" );
	
	string exeFilename = string( argv[ 0 ] );
	QDir::setCurrent( exeFilename.substr( 0, exeFilename.find_last_of( "/" ) ).c_str() );
	
	// QApplication to be used in any Qt-based tests.
	QApplication app( argc, argv );
	
	testing::InitGoogleTest( &argc, argv );
	
	// Declare the supported options.
	po::options_description desc( "Allowed test parameters" );
	desc.add_options()
		( "help", "produce help message" )
		( "octree_stress_filename", po::value< string >(), "fast parallel octree stress test's .ply filename" )
		( "octree_stress_threads", po::value< int >(), "fast parallel octree stress test's number of threads" )
		( "octree_stress_lvl", po::value< int >(), "fast parallel octree stress test's maximum hierarchy level" )
		( "octree_stress_workitem", po::value< int >(), "fast parallel octree stress test's workitem size" )
		( "octree_stress_quota", po::value< ulong >(), "fast parallel octree stress test's mwmory quota" )
	;

	po::variables_map vm;
	po::store( po::parse_command_line( argc, argv, desc ), vm );
	po::notify( vm );    

	if( vm.count( "help" ) ) {
		cout << desc << "\n";
		return 1;
	}

	if( vm.count( "octree_stress_filename" ) )
	{
		cout << "FastParallelOctreeTest params identified." << endl << endl;
		
		string plyFilename = vm[ "octree_stress_filename" ].as< string >();
		int threads = vm[ "octree_stress_threads" ].as< int >();
		int lvl = vm[ "octree_stress_lvl" ].as< int >();
		int workitem = vm[ "octree_stress_workitem" ].as< int >();
		ulong quota = vm[ "octree_stress_quota" ].as< ulong >();
		
		g_fastParallelStressParam = FastParallelOctreeTestParam( plyFilename, threads, lvl, workitem, quota );
	}
	
	return RUN_ALL_TESTS();
}
