#include "HierarchyTestMethods.h"

namespace model
{
	namespace test
	{
		void checkHierarchy( const ShallowOctreeMapPtr< OctreeNode >& hierarchy )
		{
			// Expected hierarchy. 0x1 is the root node. A node with an arrow that points to nothing means that
			// it is a sibling of the node at the same position at the line immediately above.
			//
			// 0xa6c3 -> 0x14d8 -> 0x29b -> 0x53 -> 0xa -> 0x1
			// 0xa6c0 -> 
			//								0x67 -> 0xc ->
			// 0xc325 -> 0x1864 -> 0x30c -> 0x61 ->
			// 0xc320 ->

			//								0x70 -> 0xe ->
			//							    0x71 ->
			//					   0x39f -> 0x73 ->
			//					   0x39d ->
			//			 0x1d82 -> 0x3b0 -> 0x76 ->
			//			 0x1d80 ->
			
			checkNode< ShallowMortonCode >( hierarchy, 0xa6c3U );
			checkNode< ShallowMortonCode >( hierarchy, 0xa6c0U );
			checkNode< ShallowMortonCode >( hierarchy, 0xc325U );
			checkNode< ShallowMortonCode >( hierarchy, 0xc320U );
			checkNode< ShallowMortonCode >( hierarchy, 0x1d82U );
			checkNode< ShallowMortonCode >( hierarchy, 0x1d80U );
			checkNode< ShallowMortonCode >( hierarchy, 0x39fU );
			checkNode< ShallowMortonCode >( hierarchy, 0x39dU );
			checkNode< ShallowMortonCode >( hierarchy, 0x67U );
			checkNode< ShallowMortonCode >( hierarchy, 0x61U );
			checkNode< ShallowMortonCode >( hierarchy, 0x70U );
			checkNode< ShallowMortonCode >( hierarchy, 0x71U );
			checkNode< ShallowMortonCode >( hierarchy, 0x73U );
			checkNode< ShallowMortonCode >( hierarchy, 0x76U );
			checkNode< ShallowMortonCode >( hierarchy, 0xaU );
			checkNode< ShallowMortonCode >( hierarchy, 0xcU );
			checkNode< ShallowMortonCode >( hierarchy, 0xeU );
			checkNode< ShallowMortonCode >( hierarchy, 0x1U );
			checkNode< ShallowMortonCode >( hierarchy, 0x14d8U );
			checkNode< ShallowMortonCode >( hierarchy, 0x1864U );
			checkNode< ShallowMortonCode >( hierarchy, 0x29bU );
			checkNode< ShallowMortonCode >( hierarchy, 0x30cU );
			checkNode< ShallowMortonCode >( hierarchy, 0x3b0U );
			checkNode< ShallowMortonCode >( hierarchy, 0x53U );
			
			ASSERT_TRUE( hierarchy->empty() );
		}
		
		void checkHierarchy( const MediumOctreeMapPtr< OctreeNode >& hierarchy )
		{
			// Checks if the hierarchy has the expected nodes.
			checkNode< MediumMortonCode >( hierarchy, 0xc320UL );
			checkNode< MediumMortonCode >( hierarchy, 0xc325UL );
			checkNode< MediumMortonCode >( hierarchy, 0xa6c0UL );
			checkNode< MediumMortonCode >( hierarchy, 0xa6c3UL );
			checkNode< MediumMortonCode >( hierarchy, 0x1d80UL );
			checkNode< MediumMortonCode >( hierarchy, 0x1d82UL );
			checkNode< MediumMortonCode >( hierarchy, 0x1864UL );
			checkNode< MediumMortonCode >( hierarchy, 0x14d8UL );
			checkNode< MediumMortonCode >( hierarchy, 0x39dUL );
			checkNode< MediumMortonCode >( hierarchy, 0x39fUL );
			checkNode< MediumMortonCode >( hierarchy, 0x3b0UL );
			checkNode< MediumMortonCode >( hierarchy, 0x30cUL );
			checkNode< MediumMortonCode >( hierarchy, 0x29bUL );
			checkNode< MediumMortonCode >( hierarchy, 0x73UL );
			checkNode< MediumMortonCode >( hierarchy, 0x71UL );
			checkNode< MediumMortonCode >( hierarchy, 0x70UL );
			checkNode< MediumMortonCode >( hierarchy, 0x76UL );
			checkNode< MediumMortonCode >( hierarchy, 0x67UL );
			checkNode< MediumMortonCode >( hierarchy, 0x61UL );
			checkNode< MediumMortonCode >( hierarchy, 0x53UL );
			checkNode< MediumMortonCode >( hierarchy, 0xeUL );
			checkNode< MediumMortonCode >( hierarchy, 0xcUL );
			checkNode< MediumMortonCode >( hierarchy, 0xaUL );
			checkNode< MediumMortonCode >( hierarchy, 0x1UL );
		}
	}
}