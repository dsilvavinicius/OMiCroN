#include <gtest/gtest.h>
#include "MemoryManager.h"
#include "OctreeMapTypes.h"
#include "MortonCode.h"
#include "LeafNode.h"

using namespace std;

namespace model
{
	namespace test
	{
        class MemoryManagerTest : public ::testing::Test
		{};
		
		TEST_F( MemoryManagerTest, ShallowPointVectorLeafNodes )
		{
			uint nNodes = 500000u;
			uint nPoints = 2u * nNodes;
			
			IMemoryManager& manager = MemoryManager::instance();
			
			ASSERT_EQ( manager.usedMemory(), ( size_t ) 0 );
			
			ShallowOctreeMap map;
			
			for( unsigned int i = 0u; i < nNodes; ++i )
			{
				if( i == 0.5 * nNodes )
				{
					size_t expectedHalfSize = 	0.5 * nNodes * sizeof( ShallowMortonCode )
												+ 0.5 * nPoints * sizeof( Point )
												+ 0.5 * nNodes * sizeof( ShallowLeafNode< PointVector > );
					ASSERT_EQ( manager.usedMemory(), expectedHalfSize );
				}
				
				ShallowMortonCodePtr mortonCode( new ShallowMortonCode() );
				mortonCode->build( i );
				
				ShallowLeafNodePtr< PointVector > node( new ShallowLeafNode< PointVector >() );
				PointPtr p0( new Point() );
				PointPtr p1( new Point() );
				node->setContents( PointVector( { p0, p1 } ) );
				
				map[ mortonCode ] = node;
			}
			
			size_t expectedFullSize = 	nNodes * sizeof( ShallowMortonCode ) + nPoints * sizeof( Point )
										+ nNodes * sizeof( ShallowLeafNode< PointVector > );
			ASSERT_EQ( manager.usedMemory(), expectedFullSize );
			
			map.clear();
			
			ASSERT_EQ( manager.usedMemory(), ( size_t ) 0 );
		}
		
		TEST_F( MemoryManagerTest, MediumExtendedPointVectorInnerNodes )
		{
			uint nNodes = 500000u;
			uint nPoints = 2u * nNodes;
			
			IMemoryManager& manager = MemoryManager::instance();
			
			ASSERT_EQ( manager.usedMemory(), ( size_t ) 0 );
			
			MediumOctreeMap map;
			
			for( unsigned int i = 0u; i < nNodes; ++i )
			{
				if( i == 0.5 * nNodes )
				{
					size_t expectedHalfSize = 	0.5 * nNodes * sizeof( MediumMortonCode )
												+ 0.5 * nPoints * sizeof( ExtendedPoint )
												+ 0.5 * nNodes * sizeof( MediumLeafNode< ExtendedPointVector > );
					ASSERT_EQ( manager.usedMemory(), expectedHalfSize );
				}
				
				MediumMortonCodePtr mortonCode( new MediumMortonCode() );
				mortonCode->build( i );
				
				MediumLeafNodePtr< ExtendedPointVector > node( new MediumLeafNode< ExtendedPointVector >() );
				ExtendedPointPtr p0( new ExtendedPoint() );
				ExtendedPointPtr p1( new ExtendedPoint() );
				node->setContents( ExtendedPointVector( { p0, p1 } ) );
				
				map[ mortonCode ] = node;
			}
			
			size_t expectedFullSize = 	nNodes * sizeof( MediumMortonCode ) + nPoints * sizeof( ExtendedPoint )
										+ nNodes * sizeof( MediumLeafNode< ExtendedPointVector > );
			ASSERT_EQ( manager.usedMemory(), expectedFullSize );
			
			map.clear();
			
			ASSERT_EQ( manager.usedMemory(), ( size_t ) 0 );
		}
	}
}