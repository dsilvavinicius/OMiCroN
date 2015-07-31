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
		
			vec3 origin = *octree.getOrigin();
			vec3 size = *octree.getSize();
			vec3 leafSize = *octree.getLeafSize();
		
			float epsilon = 10.e-15f;
			ASSERT_TRUE( distance2( origin, vec3( -14.f, -31.f, -51.f ) ) < epsilon );
			ASSERT_TRUE( distance2( size, vec3(60.f, 46.f, 75.f) ) < epsilon );
			ASSERT_TRUE( distance2( leafSize, vec3(0.05859375f, 0.044921875f, 0.073242188f ) ) < epsilon );
		}
		
		template< typename Octree >
		void testMediumBoundaries( const Octree& octree )
		{
			ASSERT_EQ( octree.getMaxLevel(), 20 );
			ASSERT_EQ( octree.getMaxPointsPerNode(), 1 );
			
			vec3 origin = *octree.getOrigin();
			vec3 size = *octree.getSize();
			vec3 leafSize = *octree.getLeafSize();
			
			float epsilon = 10.e-15f;
			ASSERT_TRUE( distance2( origin, vec3( -14.f, -31.f, -51.f ) )  < epsilon );
			ASSERT_TRUE( distance2( size, vec3( 60.f, 46.f, 75.f ) ) < epsilon );
			ASSERT_TRUE( distance2( leafSize, vec3( 0.00005722f, 0.000043869f, 0.000071526f ) ) < epsilon );
		}
		
		template< typename MortonCode >
		void checkNode( OctreeMapPtr< MortonCode > hierarchy, const unsigned long long& bits )
		{
			stringstream ss;
			ss << "0x" << hex << bits << dec;
			SCOPED_TRACE( ss.str() );
			shared_ptr< MortonCode > code( new MortonCode( ) );
			code->build( bits );
			auto iter = hierarchy->find( code );
			ASSERT_FALSE( iter == hierarchy->end() );
			hierarchy->erase( iter );
		}
		
		void checkHierarchy( const ShallowOctreeMapPtr& hierarchy );
		
		void checkHierarchy( const MediumOctreeMapPtr& hierarchy );
	}
}

#endif