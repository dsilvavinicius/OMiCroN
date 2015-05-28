#include <gtest/gtest.h>

#include "OutOfCoreOctree.h"

extern "C" string g_appPath;

namespace model
{
	namespace test
	{
        class OutOfCoreOctreeTest
        : public ::testing::Test
		{
		protected:
			void SetUp(){}
		};
		
		TEST_F( OutOfCoreOctreeTest, Creation )
		{
			using Point = Point< float, vec3 >;
			using OctreeNode = ShallowOctreeNode< float, vec3 >;
			
			ShallowOutOfCoreOctree< float, vec3, Point>  octree( 1, 10 );
			octree.buildFromFile( g_appPath + "/data/test_normals.ply", SimplePointReader::SINGLE, NORMALS );
			
			float epsilon = 1.e-15;
			
			SQLiteManager< Point, ShallowMortonCode, OctreeNode >& sqLite = octree.getSQLiteManager();
			
			Point* p = sqLite.getPoint( 0 );
			Point expected( vec3( 11.321565f, 4.658535f, 7.163479f ), vec3( 7.163479f, 4.658535f, 11.321565f ) );
			ASSERT_TRUE( p->equal( expected, epsilon ) );
			delete p;
			
			p = sqLite.getPoint( 1 );
			expected = Point( vec3( 11.201763f, 5.635769f, 6.996898f ), vec3( 6.996898f, 5.635769f, 11.201763f ) ); 
			ASSERT_TRUE( p->equal( expected, epsilon ) );
			delete p;
			
			p = sqLite.getPoint( 2 );
			expected = Point( vec3( 11.198129f, 4.750132f, 7.202037f ), vec3( 7.202037f, 4.750132f, 11.198129f ) ); 
			ASSERT_TRUE( p->equal( expected, epsilon ) );
			delete p;
		}
		
		TEST_F( OutOfCoreOctreeTest, CreationExtended )
		{
			using Point = ExtendedPoint< float, vec3 >;
			using OctreeNode = ShallowOctreeNode< float, vec3 >;
			
			ShallowOutOfCoreOctree< float, vec3, Point>  octree( 1, 10 );
			octree.buildFromFile( g_appPath + "/data/test_extended_points.ply", ExtendedPointReader::SINGLE,
								  COLORS_AND_NORMALS );
			
			float epsilon = 1.e-15;
			
			SQLiteManager< Point, ShallowMortonCode, OctreeNode >& sqLite = octree.getSQLiteManager();
			
			Point* p = sqLite.getPoint( 0 );
			Point expected( vec3( 0.003921569f, 0.007843137f, 0.011764706f ), vec3( 11.321565f, 4.658535f, 7.163479f ),
							vec3( 7.163479f, 4.658535f, 11.321565f ) );
			ASSERT_TRUE( p->equal( expected, epsilon ) );
			delete p;
			
			p = sqLite.getPoint( 1 );
			expected = Point( vec3( 0.015686275f, 0.019607843f, 0.023529412f ), vec3( 11.201763f, 5.635769f, 6.996898f ),
							  vec3( 6.996898f, 5.635769f, 11.201763f ) ); 
			ASSERT_TRUE( p->equal( expected, epsilon ) );
			delete p;
			
			p = sqLite.getPoint( 2 );
			expected = Point( vec3( 0.02745098f, 0.031372549f, 0.035294118f ), vec3( 11.198129f, 4.750132f, 7.202037f ),
							  vec3( 7.202037f, 4.750132f, 11.198129f ) ); 
			ASSERT_TRUE( p->equal( expected, epsilon ) );
			delete p;
		}
	}
}