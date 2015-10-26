#include <gtest/gtest.h>
#include <QApplication>

#include "HierarchyTestMethods.h"
#include "RandomSampleOctree.h"
#include <IndexedOctree.h>
#include <OutOfCoreOctree.h>
#include <MemoryManagerTypes.h>
#include "Stream.h"

namespace model
{
	namespace test
	{
		void generatePoints( PointVector& points )
		{
			// These points should define the boundaries of the octree hexahedron.
			PointPtr up = makeManaged< Point >( vec3( 0.01f, 0.02f, 0.03f ), vec3( 1.f, 15.f ,2.f ) );
			PointPtr down = makeManaged< Point >( vec3( 0.04f, 0.05f, 0.06f ), vec3( 3.f, -31.f ,4.f ) );
			PointPtr left = makeManaged< Point >( vec3( 0.07f, 0.08f, 0.09f ), vec3( -14.f, 5.f ,6.f ) );
			PointPtr right = makeManaged< Point >( vec3( 0.1f, 0.11f, 0.12f ), vec3( 46.f, 7.f ,8.f ) );
			PointPtr front = makeManaged< Point >( vec3( 0.13f, 0.14f, 0.15f ), vec3( 9.f, 10.f ,24.f ) );
			PointPtr back = makeManaged< Point >( vec3( 0.16f, 0.17f, 0.18f ), vec3( 11.f, 12.f ,-51.f ) );
			
			// Additional points inside the hexahedron.
			PointPtr addPoint0 = makeManaged< Point >( vec3( 0.19f, 0.2f, 0.21f ), vec3( 13.f, -12.f, 9.f ) );
			PointPtr addPoint1 = makeManaged< Point >( vec3( 0.22f, 0.23f, 0.24f ), vec3( -5.f, -8.f, 1.f ) );
			PointPtr addPoint2 = makeManaged< Point >( vec3( 0.25f, 0.26f, 0.27f ), vec3( 14.f, 11.f, -4.f ) );
			PointPtr addPoint3 = makeManaged< Point >( vec3( 0.28f, 0.29f, 0.30f ), vec3( 7.f, 3.f, -12.f ) );
			PointPtr addPoint4 = makeManaged< Point >( vec3( 0.31f, 0.32f, 0.33f ), vec3( 12.f, 5.f, 0.f ) );
			
			points.push_back( back );
			points.push_back( front );
			points.push_back( right );
			points.push_back( left );
			points.push_back( down );
			points.push_back( up );
			
			points.push_back( addPoint0 );
			points.push_back( addPoint1 );
			points.push_back( addPoint2 );
			points.push_back( addPoint3 );
			points.push_back( addPoint4 );
		}
		
		void generatePoints( ExtendedPointVector& points )
		{
			using Point = ExtendedPoint;
			using PointPtr = ExtendedPointPtr;
			using PointVector = ExtendedPointVector;
			
			// These points should define the boundaries of the octree hexahedron.
			PointPtr up = makeManaged< Point >( vec3( 0.01f, 0.02f, 0.03f ), vec3( 0.01f, 0.02f, 0.03f ), vec3( 1.f, 15.f ,2.f ) );
			PointPtr down = makeManaged< Point >( vec3( 0.04f, 0.05f, 0.06f ), vec3( 0.04f, 0.05f, 0.06f ), vec3( 3.f, -31.f ,4.f ) );
			PointPtr left = makeManaged< Point >( vec3( 0.07f, 0.08f, 0.09f ), vec3( 0.07f, 0.08f, 0.09f ), vec3( -14.f, 5.f ,6.f ) );
			PointPtr right = makeManaged< Point >( vec3( 0.1f, 0.11f, 0.12f ), vec3( 0.1f, 0.11f, 0.12f ), vec3( 46.f, 7.f ,8.f ) );
			PointPtr front = makeManaged< Point >( vec3( 0.13f, 0.14f, 0.15f ), vec3( 0.13f, 0.14f, 0.15f ), vec3( 9.f, 10.f ,24.f ) );
			PointPtr back = makeManaged< Point >( vec3( 0.16f, 0.17f, 0.18f ), vec3( 0.16f, 0.17f, 0.18f ), vec3( 11.f, 12.f ,-51.f ) );
			
			// Additional points inside the hexahedron.
			PointPtr addPoint0 = makeManaged< Point >( vec3( 0.19f, 0.2f, 0.21f ), vec3( 0.19f, 0.2f, 0.21f ), vec3( 13.f, -12.f, 9.f ) );
			PointPtr addPoint1 = makeManaged< Point >( vec3( 0.22f, 0.23f, 0.24f ), vec3( 0.22f, 0.23f, 0.24f ), vec3( -5.f, -8.f, 1.f ) );
			PointPtr addPoint2 = makeManaged< Point >( vec3( 0.25f, 0.26f, 0.27f ), vec3( 0.25f, 0.26f, 0.27f ), vec3( 14.f, 11.f, -4.f ) );
			PointPtr addPoint3 = makeManaged< Point >( vec3( 0.28f, 0.29f, 0.30f ), vec3( 0.28f, 0.29f, 0.30f ), vec3( 7.f, 3.f, -12.f ) );
			PointPtr addPoint4 = makeManaged< Point >( vec3( 0.31f, 0.32f, 0.33f ), vec3( 0.31f, 0.32f, 0.33f ), vec3( 12.f, 5.f, 0.f ) );
			
			points.push_back( back );
			points.push_back( front );
			points.push_back( right );
			points.push_back( left );
			points.push_back( down );
			points.push_back( up );
			
			points.push_back( addPoint0 );
			points.push_back( addPoint1 );
			points.push_back( addPoint2 );
			points.push_back( addPoint3 );
			points.push_back( addPoint4 );
		}
		
		class SimplePointTest
		: public ::testing::Test
		{
		public:
			static void SetUpTestCase()
			{
				m_plyFileName = new string( "data/simple_point_octree.ply" );
			}
		
			static void TearDownTestCase()
			{
				delete m_plyFileName;
				m_plyFileName = nullptr;
			}
			
			static string* m_plyFileName;
		};
		string* SimplePointTest::m_plyFileName;
		
		class ExtendedPointTest
		: public ::testing::Test
		{
		public:
			static void SetUpTestCase()
			{
				m_plyFileName = new string( "data/extended_point_octree.ply" );
			}
			
			static void TearDownTestCase()
			{
				delete m_plyFileName;
				m_plyFileName = nullptr;
			}
			
			static string* m_plyFileName;
		};
		string* ExtendedPointTest::m_plyFileName;
		
		template< typename P >
        class OctreeTest {};
		
		// Macro that makes one specialization of OctreeTest.
		#define SPECIALIZE_TEST(OCTREE,DERIVED_TEST,MANAGER_PREFIX,MAX_LVL) \
		template<> \
		class OctreeTest< OCTREE > \
		: public DERIVED_TEST \
		{ \
			using PointVector = OCTREE::PointVector;\
			using OctreePtr = shared_ptr< OCTREE >;\
		protected: \
			void SetUp() \
			{ \
				MANAGER_PREFIX##_DefaultManager::initInstance( 1000000 ); \
				PointVector points; \
				generatePoints( points ); \
				m_octree = OctreePtr( new OCTREE( 1, MAX_LVL ) ); \
				m_octree->build( points ); \
			} \
			\
			OctreePtr m_octree;\
		};\
		
		// This macro makes all possible template specializations for a given octree name.
		#define SPECIALIZE_ALL_TESTS_FOR_OCTREE(NAME) \
		SPECIALIZE_TEST(SPOpS_##NAME,SimplePointTest,SPV,10) \
		\
		SPECIALIZE_TEST(MPOpS_##NAME,SimplePointTest,MPV,20) \
		\
		SPECIALIZE_TEST(SEOpS_##NAME,ExtendedPointTest,SEV,10) \
		\
		SPECIALIZE_TEST(MEOpS_##NAME,ExtendedPointTest,MEV,20) \
		
		
		SPECIALIZE_ALL_TESTS_FOR_OCTREE(RandomSampleOctree)
		
		SPECIALIZE_ALL_TESTS_FOR_OCTREE(FrontOctree)
		
		
		// Defines the methods to test boundaries and hierarchy structure of a given octree type.
		#define DEFINE_TEST_FUNCTIONS(OCTREE,DEPTH) \
		void testBoundaries( const OCTREE& octree ) \
		{ \
			test##DEPTH##Boundaries( octree ); \
		} \
		\
		void testHierarchy( const OCTREE& octree ) \
		{ \
			checkHierarchy( octree.getHierarchy() ); \
		} \
		
		// Defines overloads for all possible templates for a given octree name.
		#define DEFINE_ALL_TEST_FUNCTIONS_FOR_OCTREE(NAME) \
		DEFINE_TEST_FUNCTIONS(SPOpS_##NAME,Shallow) \
		\
		DEFINE_TEST_FUNCTIONS(MPOpS_##NAME,Medium) \
		\
		DEFINE_TEST_FUNCTIONS(SEOpS_##NAME,Shallow) \
		\
		DEFINE_TEST_FUNCTIONS(MEOpS_##NAME,Medium) \
		
		
		DEFINE_ALL_TEST_FUNCTIONS_FOR_OCTREE(RandomSampleOctree)
		
		DEFINE_ALL_TEST_FUNCTIONS_FOR_OCTREE(FrontOctree)
		
		
		using testing::Types;
		
		// Lists all type names derived from an octree name.
		#define LIST_OCTREE_TYPES(NAME) \
		SPOpS_##NAME, MPOpS_##NAME, SEOpS_##NAME, MEOpS_##NAME
		
		typedef Types< 	LIST_OCTREE_TYPES(RandomSampleOctree), //LIST_OCTREE_TYPES(IndexedOctree),
						LIST_OCTREE_TYPES(FrontOctree) > Implementations;
		
		TYPED_TEST_CASE( OctreeTest, Implementations );

		/** Tests the calculated boundaries of the ShallowOctree. */
		TYPED_TEST( OctreeTest, Boundaries )
		{
			testBoundaries( *this->m_octree );
		}

		/** Tests the ShallowOctree created hierarchy. */
		TYPED_TEST( OctreeTest, Hierarchy )
		{
			testHierarchy( *this->m_octree );
		}
	}
}