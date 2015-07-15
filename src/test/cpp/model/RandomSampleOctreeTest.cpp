#include <gtest/gtest.h>

#include "RandomSampleOctree.h"

namespace model
{
	namespace test
	{
		template< typename P >
        class RandomSampleOctreeTest
        : public ::testing::Test
		{};
		
		template<>
		class RandomSampleOctreeTest< Point >
		: public ::testing::Test
		{
		protected:
			
			void SetUp(){}
		};
		
		template<>
		class RandomSampleOctreeTest< ExtendedPoint >
		: public ::testing::Test
		{
		protected:
			
			void SetUp(){}
		};

		void generatePointsInInterval( const int& numPoints, const vec2& xInterval, const vec2& yInterval,
									   const vec2& zInterval, PointVector& out_points )
		{
			for( int i = 0; i < numPoints; ++i )
			{
				float x = xInterval.x + float( rand() ) / ( float( RAND_MAX / ( xInterval.y - xInterval.x ) ) );
				float y = yInterval.x + float( rand() ) / ( float( RAND_MAX / ( yInterval.y - yInterval.x ) ) );
				float z = zInterval.x + float( rand() ) / ( float( RAND_MAX / ( zInterval.y - zInterval.x ) ) );
					
				auto point = make_shared< Point >( vec3( 1.f, 1.f, 1.f ), vec3( x, y, z ) );
				out_points.push_back( point );
			}
		}
		
		void generatePointsInInterval( const int& numPoints, const vec2& xInterval, const vec2& yInterval,
									   const vec2& zInterval, ExtendedPointVector& out_points )
		{
			for( int i = 0; i < numPoints; ++i )
			{
				float x = xInterval.x + float( rand() ) / ( float( RAND_MAX / ( xInterval.y - xInterval.x ) ) );
				float y = yInterval.x + float( rand() ) / ( float( RAND_MAX / ( yInterval.y - yInterval.x ) ) );
				float z = zInterval.x + float( rand() ) / ( float( RAND_MAX / ( zInterval.y - zInterval.x ) ) );
					
				auto point = make_shared< ExtendedPoint >( vec3( 1.f, 1.f, 1.f ), vec3( 1.f, 1.f, 1.f ),
																		  vec3( x, y, z ) );
				out_points.push_back( point );
			}
		}
		
		using testing::Types;
		
		typedef Types< Point, ExtendedPoint > Implementations;
		TYPED_TEST_CASE( RandomSampleOctreeTest, Implementations );
		
		TYPED_TEST( RandomSampleOctreeTest, Hierarchy )
		{
			using Point = TypeParam;
			using PointPtr = shared_ptr< TypeParam >;
			using PointVector = vector< PointPtr >;
			using PointVectorPtr = shared_ptr< PointVector >;
			using Octree = RandomSampleOctree< ShallowMortonCode, Point >;
			
			srand( 1 );
			PointVector points;
			int pointsPerOctant = 4;
			generatePointsInInterval( pointsPerOctant, vec2( 0.f, 15.f ), vec2( 0.f, 15.f ), vec2( 0.f, 15.f ), points );
			generatePointsInInterval( pointsPerOctant, vec2( 15.f, 30.f ), vec2( 0.f, 15.f ), vec2( 0.f, 15.f ), points );
			generatePointsInInterval( pointsPerOctant, vec2( 0.f, 15.f ), vec2( 15.f, 30.f ), vec2( 0.f, 15.f ), points );
			generatePointsInInterval( pointsPerOctant, vec2( 0.f, 15.f ), vec2( 0.f, 15.f ), vec2( 15.f, 30.f ), points );
			generatePointsInInterval( pointsPerOctant, vec2( 15.f, 30.f ), vec2( 15.f, 30.f ), vec2( 0.f, 15.f ), points );
			generatePointsInInterval( pointsPerOctant, vec2( 15.f, 30.f ), vec2( 0.f, 15.f ), vec2( 15.f, 30.f ), points );
			generatePointsInInterval( pointsPerOctant, vec2( 0.f, 15.f ), vec2( 15.f, 30.f ), vec2( 15.f, 30.f ), points );
			generatePointsInInterval( pointsPerOctant, vec2( 15.f, 30.f ), vec2( 15.f, 30.f ), vec2( 15.f, 30.f ), points );
			
			PointVector tmpPts = points;
			
			Octree octree( 1, 1 );
			octree.build( tmpPts );
			
			ShallowOctreeMapPtr hierarchy = octree.getHierarchy();
			ShallowMortonCodePtr rootCode = make_shared< ShallowMortonCode >();
			rootCode->build( 0x1 );
			
			InnerNodePtr< ShallowMortonCode, PointVector > root = dynamic_pointer_cast<
				InnerNode< ShallowMortonCode, PointVector > >( ( *hierarchy )[ rootCode ] );
			
			PointVector rootPoints = root->getContents();
			ASSERT_TRUE( rootPoints.size() == pointsPerOctant );
			
			for( PointPtr point : rootPoints )
			{
				ASSERT_TRUE( find( points.begin(), points.end(), point ) != points.end() );
			}
		}
	}
}