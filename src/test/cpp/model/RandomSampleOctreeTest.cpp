#include <gtest/gtest.h>

#include "RandomSampleOctree.h"
#include <MemoryManagerTypes.h>

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
			
			void SetUp()
			{
				SPV_DefaultManager::initInstance( 1000000 );
			}
		};
		
		template<>
		class RandomSampleOctreeTest< ExtendedPoint >
		: public ::testing::Test
		{
		protected:
			
			void SetUp()
			{
				SEV_DefaultManager::initInstance( 1000000 );
			}
		};

		void generatePointsInInterval( const int& numPoints, const Vec2& xInterval, const Vec2& yInterval,
									   const Vec2& zInterval, PointVector& out_points )
		{
			for( int i = 0; i < numPoints; ++i )
			{
				float x = xInterval.x() + float( rand() ) / ( float( RAND_MAX / ( xInterval.y() - xInterval.x() ) ) );
				float y = yInterval.x() + float( rand() ) / ( float( RAND_MAX / ( yInterval.y() - yInterval.x() ) ) );
				float z = zInterval.x() + float( rand() ) / ( float( RAND_MAX / ( zInterval.y() - zInterval.x() ) ) );
					
				PointPtr point = makeManaged< Point >( Vec3( 1.f, 1.f, 1.f ), Vec3( x, y, z ) );
				out_points.push_back( point );
			}
		}
		
		void generatePointsInInterval( const int& numPoints, const Vec2& xInterval, const Vec2& yInterval,
									   const Vec2& zInterval, ExtendedPointVector& out_points )
		{
			for( int i = 0; i < numPoints; ++i )
			{
				float x = xInterval.x() + float( rand() ) / ( float( RAND_MAX / ( xInterval.y() - xInterval.x() ) ) );
				float y = yInterval.x() + float( rand() ) / ( float( RAND_MAX / ( yInterval.y() - yInterval.x() ) ) );
				float z = zInterval.x() + float( rand() ) / ( float( RAND_MAX / ( zInterval.y() - zInterval.x() ) ) );
					
				ExtendedPointPtr point = makeManaged< ExtendedPoint >( Vec3( 1.f, 1.f, 1.f ), Vec3( 1.f, 1.f, 1.f ),
																	   Vec3( x, y, z ) );
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
			using PointVector = vector< PointPtr, ManagedAllocator< PointPtr > >;
			using PointVectorPtr = shared_ptr< PointVector >;
			using OctreeNode = model::OctreeNode< PointVector >;
			using OctreeNodePtr = shared_ptr< OctreeNode >;
			using OctreeParams = model::OctreeParams< 	ShallowMortonCode, Point, OctreeNode,
														OctreeMap< ShallowMortonCode, OctreeNode > >;
			using Octree = RandomSampleOctree< OctreeParams >;
			
			srand( 1 );
			PointVector points;
			int pointsPerOctant = 4;
			generatePointsInInterval( pointsPerOctant, Vec2( 0.f, 15.f ), Vec2( 0.f, 15.f ), Vec2( 0.f, 15.f ), points );
			generatePointsInInterval( pointsPerOctant, Vec2( 15.f, 30.f ), Vec2( 0.f, 15.f ), Vec2( 0.f, 15.f ), points );
			generatePointsInInterval( pointsPerOctant, Vec2( 0.f, 15.f ), Vec2( 15.f, 30.f ), Vec2( 0.f, 15.f ), points );
			generatePointsInInterval( pointsPerOctant, Vec2( 0.f, 15.f ), Vec2( 0.f, 15.f ), Vec2( 15.f, 30.f ), points );
			generatePointsInInterval( pointsPerOctant, Vec2( 15.f, 30.f ), Vec2( 15.f, 30.f ), Vec2( 0.f, 15.f ), points );
			generatePointsInInterval( pointsPerOctant, Vec2( 15.f, 30.f ), Vec2( 0.f, 15.f ), Vec2( 15.f, 30.f ), points );
			generatePointsInInterval( pointsPerOctant, Vec2( 0.f, 15.f ), Vec2( 15.f, 30.f ), Vec2( 15.f, 30.f ), points );
			generatePointsInInterval( pointsPerOctant, Vec2( 15.f, 30.f ), Vec2( 15.f, 30.f ), Vec2( 15.f, 30.f ), points );
			
			PointVector tmpPts = points;
			
			Octree octree( 1, 1 );
			octree.build( tmpPts );
			
			ShallowOctreeMapPtr< OctreeNode > hierarchy = octree.getHierarchy();
			ShallowMortonCodePtr rootCode = makeManaged< ShallowMortonCode >();
			rootCode->build( 0x1 );
			
			OctreeNodePtr root = ( *hierarchy )[ rootCode ];
			
			PointVector rootPoints = root->getContents();
			ASSERT_TRUE( rootPoints.size() == pointsPerOctant );
			
			for( PointPtr point : rootPoints )
			{
				ASSERT_TRUE( find( points.begin(), points.end(), point ) != points.end() );
			}
		}
	}
}