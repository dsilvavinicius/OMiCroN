#include <gtest/gtest.h>
#include <QApplication>
#include <QDir>
#include <string>
#include "MemoryManager.h"
#include <Point.h>
#include <LeafNode.h>

using namespace std;
using namespace model;

class MemoryManagedEnvironment
: public testing::Environment
{
public:
	void SetUp()
	{
		// The default manager allows this setup of allocations. This ammount of memory is enough for any currently
		// executed test. In case a new test needs more, the values here must be changed.
		ulong nNodes = 500000u;
		ulong nPoints = 2u * nNodes;
		MemoryManager::initInstance( nNodes, nNodes, nPoints, nPoints, nNodes );
	}
};

int main(int argc, char** argv)
{
	string exeFilename = string( argv[ 0 ] );
	QDir::setCurrent( exeFilename.substr( 0, exeFilename.find_last_of( "/" ) ).c_str() );
	
	// QApplication to be used in any Qt-based tests.
	QApplication app( argc, argv );
	
	testing::InitGoogleTest( &argc, argv );
	MemoryManagedEnvironment* environment = new MemoryManagedEnvironment();
	testing::AddGlobalTestEnvironment( environment );
	return RUN_ALL_TESTS();
}