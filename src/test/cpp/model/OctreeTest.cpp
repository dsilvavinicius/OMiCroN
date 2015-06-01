#include <gtest/gtest.h>
#include <QApplication>

#include "Octree.h"
#include <IndexedOctree.h>
#include "Stream.h"

extern "C" int g_argc;
extern "C" char** g_argv;
extern "C" string g_appPath;

namespace model
{
	namespace test
	{
		template< typename Point >
		struct OctreeInitializer {};
		
		template<>
		struct OctreeInitializer< Point >
		{
			using PointPtr = shared_ptr< Point >;
			using PointVector = vector< PointPtr >;
			
			static PointVector generatePoints()
			{
				// These points should define the boundaries of the octree hexahedron.
				auto up = make_shared< Point >( vec3( 0.01f, 0.02f, 0.03f ), vec3( 1.f, 15.f ,2.f ) );
				auto down = make_shared< Point >( vec3( 0.04f, 0.05f, 0.06f ), vec3( 3.f, -31.f ,4.f ) );
				auto left = make_shared< Point >( vec3( 0.07f, 0.08f, 0.09f ), vec3( -14.f, 5.f ,6.f ) );
				auto right = make_shared< Point >( vec3( 0.1f, 0.11f, 0.12f ), vec3( 46.f, 7.f ,8.f ) );
				auto front = make_shared< Point >( vec3( 0.13f, 0.14f, 0.15f ), vec3( 9.f, 10.f ,24.f ) );
				auto back = make_shared< Point >( vec3( 0.16f, 0.17f, 0.18f ), vec3( 11.f, 12.f ,-51.f ) );
				
				// Additional points inside the hexahedron.
				auto addPoint0 = make_shared< Point >( vec3( 0.19f, 0.2f, 0.21f ), vec3( 13.f, -12.f, 9.f ) );
				auto addPoint1 = make_shared< Point >( vec3( 0.22f, 0.23f, 0.24f ), vec3( -5.f, -8.f, 1.f ) );
				auto addPoint2 = make_shared< Point >( vec3( 0.25f, 0.26f, 0.27f ), vec3( 14.f, 11.f, -4.f ) );
				auto addPoint3 = make_shared< Point >( vec3( 0.28f, 0.29f, 0.30f ), vec3( 7.f, 3.f, -12.f ) );
				auto addPoint4 = make_shared< Point >( vec3( 0.31f, 0.32f, 0.33f ), vec3( 12.f, 5.f, 0.f ) );
				
				PointVector points;
				
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
				
				return points;
			}
		};
		
		template<>
		struct OctreeInitializer< ExtendedPoint >
		{
			using Point = model::ExtendedPoint;
			using PointPtr = shared_ptr< Point >;
			using PointVector = vector< PointPtr >;
			
			static PointVector generatePoints()
			{
				// These points should define the boundaries of the octree hexahedron.
				auto up = make_shared< Point >( vec3( 0.01f, 0.02f, 0.03f ), vec3( 0.01f, 0.02f, 0.03f ),
												vec3( 1.f, 15.f ,2.f ) );
				auto down = make_shared< Point >( vec3( 0.04f, 0.05f, 0.06f ), vec3( 0.04f, 0.05f, 0.06f ),
												  vec3( 3.f, -31.f ,4.f ) );
				auto left = make_shared< Point >( vec3( 0.07f, 0.08f, 0.09f ), vec3( 0.07f, 0.08f, 0.09f ),
												  vec3( -14.f, 5.f ,6.f ) );
				auto right = make_shared< Point >( vec3( 0.1f, 0.11f, 0.12f ), vec3( 0.1f, 0.11f, 0.12f ),
												   vec3( 46.f, 7.f ,8.f ) );
				auto front = make_shared< Point >( vec3( 0.13f, 0.14f, 0.15f ), vec3( 0.13f, 0.14f, 0.15f ),
												   vec3( 9.f, 10.f ,24.f ) );
				auto back = make_shared< Point >( vec3( 0.16f, 0.17f, 0.18f ), vec3( 0.16f, 0.17f, 0.18f ),
												  vec3( 11.f, 12.f ,-51.f ) );
				
				// Additional points inside the hexahedron.
				auto addPoint0 = make_shared< Point >( vec3( 0.19f, 0.2f, 0.21f ), vec3( 0.19f, 0.2f, 0.21f ),
													   vec3( 13.f, -12.f, 9.f ) );
				auto addPoint1 = make_shared< Point >( vec3( 0.22f, 0.23f, 0.24f ), vec3( 0.22f, 0.23f, 0.24f ),
													   vec3( -5.f, -8.f, 1.f ) );
				auto addPoint2 = make_shared< Point >( vec3( 0.25f, 0.26f, 0.27f ), vec3( 0.25f, 0.26f, 0.27f ),
													   vec3( 14.f, 11.f, -4.f ) );
				auto addPoint3 = make_shared< Point >( vec3( 0.28f, 0.29f, 0.30f ), vec3( 0.28f, 0.29f, 0.30f ),
													   vec3( 7.f, 3.f, -12.f ) );
				auto addPoint4 = make_shared< Point >( vec3( 0.31f, 0.32f, 0.33f ), vec3( 0.31f, 0.32f, 0.33f ),
													   vec3( 12.f, 5.f, 0.f ) );
				
				PointVector points;
				
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
				
				return points;
			}
		};
		
		class SimplePointTest
		: public ::testing::Test
		{
		public:
			static void SetUpTestCase()
			{
				string exeFilename = string( g_argv[ 0 ] );
				m_plyFileName = new string( g_appPath +
					"/../../../src/data/tests/simple_point_octree.ply" );
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
				string exeFilename = string( g_argv[ 0 ] );
				m_plyFileName = new string( exeFilename.substr( 0, exeFilename.find_last_of( "/" ) ) +
					"/../../../src/data/tests/extended_point_octree.ply" );
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
		
		using ShallowPointOctree = model::Octree< ShallowMortonCode, Point >;
		using MediumPointOctree = model::Octree< MediumMortonCode, Point >;
		using ShallowExtendedPointOctree = model::Octree< ShallowMortonCode, ExtendedPoint >;
		using MediumExtendedPointOctree = model::Octree< MediumMortonCode, ExtendedPoint >;
		using ShallowExtendedIndexedOctree = model::IndexedOctree< ShallowMortonCode, ExtendedPoint >;
		
		template<>
		class OctreeTest< ShallowPointOctree >
		: public SimplePointTest
		{
			using PointVector = vector< shared_ptr< Point > >;
			using Octree = ShallowPointOctree;
			using OctreePtr = shared_ptr< Octree >;
			using Test = model::test::OctreeTest< Octree >;
			using OctreeInitializer = model::test::OctreeInitializer< Point >;
			
		public:
			friend OctreeInitializer;
			
		protected:
			void SetUp()
			{
				PointVector points = OctreeInitializer::generatePoints();
				m_octree = make_shared< Octree >( 1, 10 );
				m_octree->build( points );
			}
			
			OctreePtr m_octree; 
		};
		
		template<>
		class OctreeTest< MediumPointOctree >
		: public SimplePointTest
		{
			using PointVector = vector< shared_ptr< Point > >;
			using Octree = MediumPointOctree;
			using OctreePtr = shared_ptr< Octree >;
			using Test = model::test::OctreeTest< Octree >;
			using OctreeInitializer = model::test::OctreeInitializer< Point >;
			
		public:
			friend OctreeInitializer;
			
		protected:
			void SetUp()
			{
				PointVector points = OctreeInitializer::generatePoints();
				m_octree = make_shared< Octree >( 1, 20 );
				m_octree->build( points );
			}
			
			OctreePtr m_octree; 
		};
		
		template<>
		class OctreeTest< ShallowExtendedPointOctree >
		: public ExtendedPointTest
		{
			using Point = ExtendedPoint;
			using PointVector = vector< shared_ptr< Point > >;
			using Octree = ShallowExtendedPointOctree;
			using OctreePtr = shared_ptr< Octree >;
			using Test = model::test::OctreeTest< Octree >;
			using OctreeInitializer = model::test::OctreeInitializer< Point >;
		
		public:
			friend OctreeInitializer;
			
		protected:
			/** Creates points that will be inside the octree and the associated expected results of octree construction. */
			void SetUp()
			{
				PointVector points = OctreeInitializer::generatePoints();
				m_octree = make_shared< Octree >( 1, 10 );
				m_octree->build( points );
			}
			
			OctreePtr m_octree;
		};
		
		template<>
		class OctreeTest< MediumExtendedPointOctree >
		: public ExtendedPointTest
		{
			using Point = ExtendedPoint;
			using PointVector = vector< shared_ptr< Point > >;
			using Octree = MediumExtendedPointOctree;
			using OctreePtr = shared_ptr< Octree >;
			using Test = model::test::OctreeTest< Octree >;
			using OctreeInitializer = model::test::OctreeInitializer< Point >;
		
		public:
			friend OctreeInitializer;
			
		protected:
			/** Creates points that will be inside the octree and the associated expected results of octree construction. */
			void SetUp()
			{
				PointVector points = OctreeInitializer::generatePoints();
				m_octree = make_shared< Octree >( 1, 20 );
				m_octree->build( points );
			}
			
			OctreePtr m_octree;
		};
		
		template<>
		class OctreeTest< ShallowExtendedIndexedOctree >
		: public ExtendedPointTest
		{
			using Point = ExtendedPoint;
			using PointVector = vector< shared_ptr< Point > >;
			using Octree = ShallowExtendedIndexedOctree;
			using OctreePtr = shared_ptr< Octree >;
			using Test = model::test::OctreeTest< Octree >;
			using OctreeInitializer = model::test::OctreeInitializer< Point >;
		
		public:
			friend OctreeInitializer;
			
		protected:
			/** Creates points that will be inside the octree and the associated expected results of octree construction. */
			void SetUp()
			{
				PointVector points = OctreeInitializer::generatePoints();
				m_octree = make_shared< Octree >( 1, 10 );
				m_octree->build( points );
			}
			
			OctreePtr m_octree;
		};
		
		template< typename Octree >
		void testShallowBoundaries( const Octree& octree )
		{
			ASSERT_EQ( octree.getMaxLevel(), 10 );
			ASSERT_EQ( octree.getMaxPointsPerNode(), 1 );
		
			vec3 origin = *octree.getOrigin();
			vec3 size = *octree.getSize();
			vec3 leafSize = *octree.getLeafSize();
		
			float epsilon = 10.e-15f;
			ASSERT_TRUE( distance2( origin, vec3( -14.f, -31.f, -51.f ) ) < epsilon ) ;
			ASSERT_TRUE( distance2( size, vec3(60.f, 46.f, 75.f) ) < epsilon );
			ASSERT_TRUE( distance2( leafSize, vec3(0.05859375f, 0.044921875f, 0.073242188f ) ) < epsilon ) ;
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
			auto code = make_shared< MortonCode >();
			code->build( bits );
			auto iter = hierarchy->find(code);
			ASSERT_FALSE( iter == hierarchy->end() );
			hierarchy->erase( iter );
		}
		
		void checkSparseHierarchy( ShallowOctreeMapPtr hierarchy )
		{
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
		}
		
		void checkHierarchy( const ShallowOctreeMapPtr& hierarchy )
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
			
			// Check node that should appear in the sparse representation of the octree.
			checkSparseHierarchy( hierarchy );
			
			// Check the other nodes, which would be merged in a sparse octree.
			checkNode< ShallowMortonCode >( hierarchy, 0x14d8U );
			checkNode< ShallowMortonCode >( hierarchy, 0x1864U );
			checkNode< ShallowMortonCode >( hierarchy, 0x29bU );
			checkNode< ShallowMortonCode >( hierarchy, 0x30cU );
			checkNode< ShallowMortonCode >( hierarchy, 0x3b0U );
			checkNode< ShallowMortonCode >( hierarchy, 0x53U );
			
			ASSERT_TRUE( hierarchy->empty() );
		}
		
		void checkHierarchy( const MediumOctreeMapPtr& hierarchy )
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
		
		template< typename Octree >
		struct OctreeTester {};
		
		template<>
		struct OctreeTester< ShallowPointOctree  >
		{
			static void testBoundaries( const ShallowPointOctree& octree )
			{
				testShallowBoundaries( octree );
			}
			
			static void testHierarchy( const ShallowPointOctree& octree )
			{
				checkHierarchy( octree.getHierarchy() );
			}
		};
		
		template<>
		struct OctreeTester< MediumPointOctree  >
		{
			static void testBoundaries( const MediumPointOctree& octree )
			{
				testMediumBoundaries( octree );
			}
			
			static void testHierarchy( const MediumPointOctree& octree )
			{
				checkHierarchy( octree.getHierarchy() );
			}
		};
		
		template<>
		struct OctreeTester< ShallowExtendedPointOctree  >
		{
			static void testBoundaries( const ShallowExtendedPointOctree& octree )
			{
				testShallowBoundaries( octree );
			}
			
			static void testHierarchy( const ShallowExtendedPointOctree& octree )
			{
				checkHierarchy( octree.getHierarchy() );
			}
		};
		
		template<>
		struct OctreeTester< MediumExtendedPointOctree  >
		{
			static void testBoundaries( const MediumExtendedPointOctree& octree )
			{
				testMediumBoundaries( octree );
			}
			
			static void testHierarchy( const MediumExtendedPointOctree& octree )
			{
				checkHierarchy( octree.getHierarchy() );
			}
		};
		
		template<>
		struct OctreeTester< ShallowExtendedIndexedOctree  >
		{
			static void testBoundaries( const ShallowExtendedIndexedOctree& octree )
			{
				testShallowBoundaries( octree );
			}
			
			static void testHierarchy( const ShallowExtendedIndexedOctree& octree )
			{
				checkHierarchy( octree.getHierarchy() );
			}
		};
		
		using testing::Types;
		
		typedef Types< ShallowPointOctree, ShallowExtendedPointOctree, MediumPointOctree, MediumExtendedPointOctree,
					   ShallowExtendedIndexedOctree > Implementations;
		TYPED_TEST_CASE( OctreeTest, Implementations );

		/** Tests the calculated boundaries of the ShallowOctree. */
		TYPED_TEST( OctreeTest, Boundaries )
		{
			OctreeTester< TypeParam >::testBoundaries( *this->m_octree );
		}

		/** Tests the ShallowOctree created hierarchy. */
		TYPED_TEST( OctreeTest, Hierarchy )
		{
			OctreeTester< TypeParam >::testHierarchy( *this->m_octree );
		}
		
		// TODO Make these tests viable again ( typed tests cannot receive value parameters right now ).
		/** Tests the ShallowOctree created hierarchy, reading from .ply file. */
		/**TYPED_TEST( OctreeTest, ShallowHierarchyFromFile )
		{
			//See ShallowHierarchy for details.
			auto octree = make_shared< ShallowOctree< float, vec3, TypeParam > >(1, 10);
			
			cout << "ShallowHierarchyFromFile filename: " << *( this->m_plyFileName ) << endl;
			
			octree->build( *( this->m_plyFileName ), PlyPointReader<float, vec3, TypeParam>::SINGLE,
						   Attributes::COLORS );
			
			checkShallowHierarchy( octree );
		}*/
	}
}