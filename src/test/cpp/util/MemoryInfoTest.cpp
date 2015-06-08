#include "MemoryInfo.h"
#include <gtest/gtest.h>
#include <thread>
#include <chrono>  

using namespace std;

namespace util
{
	namespace test
	{
        class MemoryInfoTest : public ::testing::Test
		{};

		TEST_F( MemoryInfoTest, API )
		{
			unsigned long memorySize = MemoryInfo::getMemorySize();
			unsigned long availableMemorySize = MemoryInfo::getAvailableMemorySize();
			
			cout << "Physical memory size: " << memorySize << endl
				 << "Available memory size: " << availableMemorySize << endl;
			
			ASSERT_NE( memorySize, 0uL );
			ASSERT_NE( availableMemorySize, 0uL );
			ASSERT_LT( availableMemorySize, memorySize );
		}
	}
}