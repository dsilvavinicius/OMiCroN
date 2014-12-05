#include <gtest/gtest.h>

int g_argc;
char** g_argv;

int main(int argc, char** argv) {
	g_argc = argc;
	g_argv = argv;
	
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}