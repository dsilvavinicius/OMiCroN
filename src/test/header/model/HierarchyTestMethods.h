#ifndef HIERARCHY_TEST_METHODS_H
#define HIERARCHY_TEST_METHODS_H

#include <gtest/gtest.h>
#include "MortonCode.h"
#include "OctreeMapTypes.h"

/**
 * Methods to test hierarchies generated using points in test/data/simple_point_octree.ply or
 * test/data/extended_point_octree.ply. The octree can be Shallow or Medium.
 */
namespace model
{
	namespace test
	{
		template< typename Octree >
		void testShallowBoundaries( const Octree& octree )
		{
			ASSERT_EQ( octree.getMaxLevel(), 10 );
			ASSERT_EQ( octree.getMaxPointsPerNode(), 1 );
		
			Vec3 origin = *octree.getOrigin();
			Vec3 size = *octree.getSize();
			Vec3 leafSize = *octree.getLeafSize();
		
			float epsilon = 10.e-15f;
			ASSERT_TRUE( origin.isApprox( Vec3( -14.f, -31.f, -51.f ), epsilon ) );
			ASSERT_TRUE( size.isApprox( Vec3( 60.f, 46.f, 75.f ), epsilon ) );
			ASSERT_TRUE( leafSize.isApprox( Vec3( 0.05859375f, 0.044921875f, 0.073242188f ), epsilon ) );
		}
		
		template< typename Octree >
		void testMediumBoundaries( const Octree& octree )
		{
			ASSERT_EQ( octree.getMaxLevel(), 20 );
			ASSERT_EQ( octree.getMaxPointsPerNode(), 1 );
			
			Vec3 origin = *octree.getOrigin();
			Vec3 size = *octree.getSize();
			Vec3 leafSize = *octree.getLeafSize();
			
			float epsilon = 10.e-15f;
			ASSERT_TRUE( origin.isApprox( Vec3( -14.f, -31.f, -51.f ), epsilon ) );
			ASSERT_TRUE( size.isApprox( Vec3( 60.f, 46.f, 75.f ), epsilon ) );
			ASSERT_TRUE( leafSize.isApprox( Vec3( 0.00005722f, 0.000043869f, 0.000071526f ), epsilon ) );
		}
		
		template< typename MortonCode, typename OctreeNode >
		void checkNode( OctreeMapPtr< MortonCode, OctreeNode > hierarchy, const unsigned long long& bits )
		{
			shared_ptr< MortonCode > code = makeManaged< MortonCode >( );
			code->build( bits );
			SCOPED_TRACE( code->getPathToRoot( true ) );
			auto iter = hierarchy->find( code );
			ASSERT_FALSE( iter == hierarchy->end() );
			hierarchy->erase( iter );
		}
		
		template< typename OctreeNode >
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
		
		template< typename OctreeNode >
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

#endif