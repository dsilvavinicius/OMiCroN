#include <gtest/gtest.h>

#include "RandomSampleOctree.h"

namespace model
{
	namespace test
	{
        class RandomSampleOctreeTest : public ::testing::Test
		{
		protected:
			
			void SetUp(){}
		};

		void generatePointsInInterval( const int& numPoints, const vec2& xInterval, const vec2& yInterval,
									   const vec2& zInterval, PointVector< float, vec3 >& out_points )
		{
			for( int i = 0; i < numPoints; ++i )
			{
				float x = xInterval.x + float( rand() ) / ( float( RAND_MAX / ( xInterval.y - xInterval.x ) ) );
				float y = yInterval.x + float( rand() ) / ( float( RAND_MAX / ( yInterval.y - yInterval.x ) ) );
				float z = zInterval.x + float( rand() ) / ( float( RAND_MAX / ( zInterval.y - zInterval.x ) ) );
					
				auto point = make_shared< Point< float, vec3 > >( vec3( 1.f, 1.f, 1.f ), vec3( x, y, z ) );
				out_points.push_back( point );
			}
		}
		
		TEST_F( RandomSampleOctreeTest, Hierarchy )
		{
			srand( 1 );
			PointVector< float, vec3 > points;
			int pointsPerOctant = 4;
			generatePointsInInterval( pointsPerOctant, vec2( 0.f, 15.f ), vec2( 0.f, 15.f ), vec2( 0.f, 15.f ), points );
			generatePointsInInterval( pointsPerOctant, vec2( 15.f, 30.f ), vec2( 0.f, 15.f ), vec2( 0.f, 15.f ), points );
			generatePointsInInterval( pointsPerOctant, vec2( 0.f, 15.f ), vec2( 15.f, 30.f ), vec2( 0.f, 15.f ), points );
			generatePointsInInterval( pointsPerOctant, vec2( 0.f, 15.f ), vec2( 0.f, 15.f ), vec2( 15.f, 30.f ), points );
			generatePointsInInterval( pointsPerOctant, vec2( 15.f, 30.f ), vec2( 15.f, 30.f ), vec2( 0.f, 15.f ), points );
			generatePointsInInterval( pointsPerOctant, vec2( 15.f, 30.f ), vec2( 0.f, 15.f ), vec2( 15.f, 30.f ), points );
			generatePointsInInterval( pointsPerOctant, vec2( 0.f, 15.f ), vec2( 15.f, 30.f ), vec2( 15.f, 30.f ), points );
			generatePointsInInterval( pointsPerOctant, vec2( 15.f, 30.f ), vec2( 15.f, 30.f ), vec2( 15.f, 30.f ), points );
			
			ShallowRandomSampleOctree< float, vec3, Point< float, vec3 > > octree( 1, 1 );
			octree.build( points );
			
			ShallowOctreeMapPtr< float, vec3 > hierarchy = octree.getHierarchy();
			ShallowMortonCodePtr rootCode = make_shared< ShallowMortonCode >();
			rootCode->build( 0x1 );
			
			RandomInnerNodePtr< unsigned int, float, vec3 > root = dynamic_pointer_cast<
				RandomInnerNode< unsigned int, float, vec3 > >( ( *hierarchy )[ rootCode ] );
			
			PointVectorPtr< float, vec3 > rootPoints = root->getContents();
			ASSERT_TRUE( rootPoints->size() == pointsPerOctant );
			
			for( PointPtr< float, vec3 > point : *rootPoints )
			{
				ASSERT_TRUE( find( points.begin(), points.end(), point ) != points.end() );
			}
		}
	}
}