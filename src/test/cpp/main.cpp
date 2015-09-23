#include <gtest/gtest.h>
#include <QApplication>
#include <QDir>
#include <string>
#include "MemoryManager.h"
#include <Point.h>
#include <LeafNode.h>
#include <BitMapMemoryManager.h>

using namespace std;
using namespace model;

int main(int argc, char** argv)
{
	string exeFilename = string( argv[ 0 ] );
	QDir::setCurrent( exeFilename.substr( 0, exeFilename.find_last_of( "/" ) ).c_str() );
	
	// QApplication to be used in any Qt-based tests.
	QApplication app( argc, argv );
	
	testing::InitGoogleTest( &argc, argv );
	return RUN_ALL_TESTS();
}