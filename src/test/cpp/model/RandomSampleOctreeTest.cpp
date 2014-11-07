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
		class RandomSampleOctreeTest< Point<float, vec3> >
		: public ::testing::Test
		{
		protected:
			
			void SetUp(){}
		};
		
		template<>
		class RandomSampleOctreeTest< ExtendedPoint<float, vec3> >
		: public ::testing::Test
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
		
		void generatePointsInInterval( const int& numPoints, const vec2& xInterval, const vec2& yInterval,
									   const vec2& zInterval, ExtendedPointVector< float, vec3 >& out_points )
		{
			for( int i = 0; i < numPoints; ++i )
			{
				float x = xInterval.x + float( rand() ) / ( float( RAND_MAX / ( xInterval.y - xInterval.x ) ) );
				float y = yInterval.x + float( rand() ) / ( float( RAND_MAX / ( yInterval.y - yInterval.x ) ) );
				float z = zInterval.x + float( rand() ) / ( float( RAND_MAX / ( zInterval.y - zInterval.x ) ) );
					
				auto point = make_shared< ExtendedPoint< float, vec3 > >( vec3( 1.f, 1.f, 1.f ), vec3( 1.f, 1.f, 1.f ),
																		  vec3( x, y, z ) );
				out_points.push_back( point );
			}
		}
		
		using testing::Types;
		
		typedef Types< Point< float, vec3 >, ExtendedPoint< float, vec3 > > Implementations;
		TYPED_TEST_CASE( RandomSampleOctreeTest, Implementations );
		
		TYPED_TEST( RandomSampleOctreeTest, Hierarchy )
		{
			using Point = TypeParam;
			using PointPtr = shared_ptr< TypeParam >;
			using PointVector = vector< PointPtr >;
			using PointVectorPtr = shared_ptr< PointVector >;
			
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
			
			ShallowRandomSampleOctree< float, vec3, Point > octree( 1, 1 );
			octree.build( points );
			
			ShallowOctreeMapPtr< float, vec3 > hierarchy = octree.getHierarchy();
			ShallowMortonCodePtr rootCode = make_shared< ShallowMortonCode >();
			rootCode->build( 0x1 );
			
			InnerNodePtr< unsigned int, float, vec3, PointVector > root = dynamic_pointer_cast<
				InnerNode< unsigned int, float, vec3, PointVector > >( ( *hierarchy )[ rootCode ] );
			
			PointVectorPtr rootPoints = root->getContents();
			ASSERT_TRUE( rootPoints->size() == pointsPerOctant );
			
			for( PointPtr point : *rootPoints )
			{
				ASSERT_TRUE( find( points.begin(), points.end(), point ) != points.end() );
			}
		}
	}
}