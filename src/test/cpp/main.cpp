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
		// The default manager allows this setup of allocations. This ammount of memory is enough for any currently
		// executed test. In case a new test needs more, the values here must be changed.
		MemoryManager::initInstance( 20709060u, 20709060u, 41418120u, 41418120u, 20709060u );
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