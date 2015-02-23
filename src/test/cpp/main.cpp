#include <gtest/gtest.h>

#include <QApplication>
#include <string>

using namespace std;

int g_argc;
char** g_argv;

// Path where the executable is in.
string g_appPath;

int main(int argc, char** argv)
{
	g_argc = argc;
	g_argv = argv;
	string exeFilename = string( g_argv[ 0 ] );
	g_appPath = exeFilename.substr( 0, exeFilename.find_last_of( "/" ) );
	
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}