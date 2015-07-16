#include <gtest/gtest.h>
#include "MemoryManager.h"

extern "C" string g_appPath;

namespace model
{
	namespace test
	{
        class MemoryManagerTest
        : public ::testing::Test
		{
		protected:
			void SetUp(){}
		};
		
		TEST_F( MemoryManagerTest, PointsAndNodes )
		{
		}
	}
}