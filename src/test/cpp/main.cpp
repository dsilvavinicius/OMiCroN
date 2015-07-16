#include <gtest/gtest.h>

#include <QApplication>
#include <string>
#include "MemoryManager.h"
#include <Point.h>
#include <LeafNode.h>

using namespace std;
using namespace model;

int g_argc;
char** g_argv;

// Path where the executable is in.
string g_appPath;

class MemoryManagedEnvironment
: public testing::Environment
{
public:
	void SetUp()
	{
		// The default manager allows allocations of 100 entities of each type.
		MemoryManager::initInstance( 100, 100, 20709060, 20709060, 20709060 );
	}
};

int main(int argc, char** argv)
{
	g_argc = argc;
	g_argv = argv;
	string exeFilename = string( g_argv[ 0 ] );
	g_appPath = exeFilename.substr( 0, exeFilename.find_last_of( "/" ) );
	
	testing::InitGoogleTest( &argc, argv );
	MemoryManagedEnvironment* environment = new MemoryManagedEnvironment();
	testing::AddGlobalTestEnvironment( environment );
	return RUN_ALL_TESTS();
}