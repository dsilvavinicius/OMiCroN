#include <gtest/gtest.h>

#include "RandomSampleOctree.h"

namespace model
{
	namespace test
	{
        /*class RandomSampleOctreeTest : public ::testing::Test
		{
		protected:
			
			void SetUp()
			{
				// These points should define the boundaries of the octree hexahedron.
				auto up = make_shared< Point< float, vec3 > >( vec3( 0.01f, 0.02f, 0.03f ), vec3( 1.f, 15.f ,2.f ) );
				auto down = make_shared< Point< float, vec3 > >( vec3( 0.04f, 0.05f, 0.06f ), vec3( 3.f, -31.f ,4.f ) );
				auto left = make_shared< Point< float, vec3 > >( vec3( 0.07f, 0.08f, 0.09f ), vec3( -14.f, 5.f ,6.f ) );
				auto right = make_shared< Point< float, vec3 > >( vec3( 0.1f, 0.11f, 0.12f ), vec3( 46.f, 7.f ,8.f ) );
				auto front = make_shared< Point< float, vec3 > >( vec3( 0.13f, 0.14f, 0.15f ), vec3( 9.f, 10.f ,24.f ) );
				auto back = make_shared< Point< float, vec3 > >( vec3( 0.16f, 0.17f, 0.18f ), vec3( 11.f, 12.f ,-51.f ) );
				
				// Additional points inside the hexahedron.
				auto addPoint0 = make_shared< Point< float, vec3 > >( vec3( 0.19f, 0.2f, 0.21f ), vec3( 13.f, -12.f, 9.f ) );
				auto addPoint1 = make_shared< Point< float, vec3 > >( vec3( 0.22f, 0.23f, 0.24f ), vec3( -5.f, -8.f, 1.f ) );
				auto addPoint2 = make_shared< Point< float, vec3 > >( vec3( 0.25f, 0.26f, 0.27f ), vec3( 14.f, 11.f, -4.f ) );
				auto addPoint3 = make_shared< Point< float, vec3 > >( vec3( 0.28f, 0.29f, 0.30f ), vec3( 7.f, 3.f, -12.f ) );
				auto addPoint4 = make_shared< Point< float, vec3 > >( vec3( 0.31f, 0.32f, 0.33f ), vec3( 12.f, 5.f, 0.f ) );
				
				m_points.push_back( back );
				m_points.push_back( front );
				m_points.push_back( right );
				m_points.push_back( left );
				m_points.push_back( down );
				m_points.push_back( up );
				
				m_points.push_back( addPoint0 );
				m_points.push_back( addPoint1 );
				m_points.push_back( addPoint2 );
				m_points.push_back( addPoint3 );
				m_points.push_back( addPoint4 );
			}
			
			PointVector< float, vec3 > m_points;
		};

		TEST_F( RandomSampleOctreeTest, Hierarchy )
		{
			
		}*/
	}
}