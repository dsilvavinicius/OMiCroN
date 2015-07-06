#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include "MemoryInfo.h"
#include "OctreeMapTypes.h"
#include "MortonCode.h"
#include "LeafNode.h"

using namespace model;
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
		
		TEST_F( MemoryInfoTest, StressNodes )
		{
			ShallowOctreeMap map;
			
			cout << "Available mem before allocs (MB):" << MemoryInfo::getAvailableMemorySize() / ( 1024 * 1024 )
				 << endl;
			
			for( unsigned int i = 0x1u; i < 0x13bfec4u; ++i )
			{
				ShallowMortonCodePtr mortonCode( new ShallowMortonCode() );
				mortonCode->build( i ); 
				
				ShallowLeafNodePtr< PointVector > node( new ShallowLeafNode< PointVector >() );
				PointPtr p0( new Point() );
				PointPtr p1( new Point() );
				node->setContents( PointVector( { p0, p1 } ) );
				
				map[ mortonCode ] = node;
			}
			
			cout << "Available mem after allocs (MB): " << MemoryInfo::getAvailableMemorySize() / ( 1024 * 1024 )
				 << endl;
			
			map.clear();
			
			cout << "Available after clear (MB): " << MemoryInfo::getAvailableMemorySize() / ( 1024 * 1024 ) << endl;
		}
	}
}