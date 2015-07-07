#include <gtest/gtest.h>
#include <QApplication>

#include "Octree.h"
#include <IndexedOctree.h>
#include <OutOfCoreOctree.h>
#include "Stream.h"

extern "C" string g_appPath;

namespace model
{
	namespace test
	{
		void generatePoints( PointVector& points )
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
				m_plyFileName = new string( g_appPath + "/data/simple_point_octree.ply" );
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
				m_plyFileName = new string( g_appPath + "/data/extended_point_octree.ply" );
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
		
		template<>
		class OctreeTest< ShallowOctree >
		: public SimplePointTest
		{
			using Octree = ShallowOctree;
			using OctreePtr = ShallowOctreePtr;
			using Test = model::test::OctreeTest< Octree >;
			
		protected:
			void SetUp()
			{
				PointVector points;
				generatePoints( points );
				m_octree = make_shared< Octree >( 1, 10 );
				m_octree->build( points );
			}
			
			OctreePtr m_octree; 
		};
		
		template<>
		class OctreeTest< MediumOctree >
		: public SimplePointTest
		{
			using Octree = MediumOctree;
			using OctreePtr = MediumOctreePtr;
			using Test = model::test::OctreeTest< Octree >;
			
		protected:
			void SetUp()
			{
				PointVector points;
				generatePoints( points );
				m_octree = make_shared< Octree >( 1, 20 );
				m_octree->build( points );
			}
			
			OctreePtr m_octree; 
		};
		
		template<>
		class OctreeTest< ShallowExtOctree >
		: public ExtendedPointTest
		{
			using Point = ExtendedPoint;
			using PointVector = ExtendedPointVector;
			using Octree = ShallowExtOctree;
			using OctreePtr = ShallowExtOctreePtr;
			using Test = model::test::OctreeTest< Octree >;
		
		protected:
			/** Creates points that will be inside the octree and the associated expected results of octree construction. */
			void SetUp()
			{
				PointVector points;
				generatePoints( points );
				m_octree = make_shared< Octree >( 1, 10 );
				m_octree->build( points );
			}
			
			OctreePtr m_octree;
		};
		
		template<>
		class OctreeTest< MediumExtOctree >
		: public ExtendedPointTest
		{
			using Point = ExtendedPoint;
			using PointVector = ExtendedPointVector;
			using Octree = MediumExtOctree;
			using OctreePtr = MediumExtOctreePtr;
			using Test = model::test::OctreeTest< Octree >;
		
		protected:
			/** Creates points that will be inside the octree and the associated expected results of octree construction. */
			void SetUp()
			{
				PointVector points;
				generatePoints( points );
				m_octree = make_shared< Octree >( 1, 20 );
				m_octree->build( points );
			}
			
			OctreePtr m_octree;
		};
		
		template<>
		class OctreeTest< ShallowRandomSampleOctree >
		: public SimplePointTest
		{
			using Octree = ShallowRandomSampleOctree;
			using OctreePtr = ShallowRandomSampleOctreePtr;
			using Test = model::test::OctreeTest< Octree >;
		
		protected:
			/** Creates points that will be inside the octree and the associated expected results of octree construction. */
			void SetUp()
			{
				PointVector points;
				generatePoints( points );
				m_octree = make_shared< Octree >( 1, 10 );
				m_octree->build( points );
			}
			
			OctreePtr m_octree;
		};
		
		template<>
		class OctreeTest< MediumRandomSampleOctree >
		: public SimplePointTest
		{
			using Octree = MediumRandomSampleOctree;
			using OctreePtr = MediumRandomSampleOctreePtr;
			using Test = model::test::OctreeTest< Octree >;
		
		protected:
			/** Creates points that will be inside the octree and the associated expected results of octree construction. */
			void SetUp()
			{
				PointVector points;
				generatePoints( points );
				m_octree = make_shared< Octree >( 1, 20 );
				m_octree->build( points );
			}
			
			OctreePtr m_octree;
		};
		
		template<>
		class OctreeTest< ShallowExtRandomSampleOctree >
		: public ExtendedPointTest
		{
			using Point = ExtendedPoint;
			using PointVector = ExtendedPointVector;
			using Octree = ShallowExtRandomSampleOctree;
			using OctreePtr = ShallowExtRandomSampleOctreePtr;
			using Test = model::test::OctreeTest< Octree >;
		
		protected:
			/** Creates points that will be inside the octree and the associated expected results of octree construction. */
			void SetUp()
			{
				PointVector points;
				generatePoints( points );
				m_octree = make_shared< Octree >( 1, 10 );
				m_octree->build( points );
			}
			
			OctreePtr m_octree;
		};
		
		template<>
		class OctreeTest< MediumExtRandomSampleOctree >
		: public ExtendedPointTest
		{
			using Point = ExtendedPoint;
			using PointVector = ExtendedPointVector;
			using Octree = MediumExtRandomSampleOctree;
			using OctreePtr = MediumExtRandomSampleOctreePtr;
			using Test = model::test::OctreeTest< Octree >;
		
		protected:
			/** Creates points that will be inside the octree and the associated expected results of octree construction. */
			void SetUp()
			{
				PointVector points;
				generatePoints( points );
				m_octree = make_shared< Octree >( 1, 20 );
				m_octree->build( points );
			}
			
			OctreePtr m_octree;
		};
		
		template<>
		class OctreeTest< ShallowIndexedOctree >
		: public SimplePointTest
		{
			using Octree = ShallowIndexedOctree;
			using OctreePtr = ShallowIndexedOctreePtr;
			using Test = model::test::OctreeTest< Octree >;
		
		protected:
			/** Creates points that will be inside the octree and the associated expected results of octree construction. */
			void SetUp()
			{
				PointVector points;
				generatePoints( points );
				m_octree = make_shared< Octree >( 1, 10 );
				m_octree->build( points );
			}
			
			OctreePtr m_octree;
		};
		
		template<>
		class OctreeTest< MediumIndexedOctree >
		: public SimplePointTest
		{
			using Octree = MediumIndexedOctree;
			using OctreePtr = MediumIndexedOctreePtr;
			using Test = model::test::OctreeTest< Octree >;
		
		protected:
			/** Creates points that will be inside the octree and the associated expected results of octree construction. */
			void SetUp()
			{
				PointVector points;
				generatePoints( points );
				m_octree = make_shared< Octree >( 1, 20 );
				m_octree->build( points );
			}
			
			OctreePtr m_octree;
		};
		
		template<>
		class OctreeTest< ShallowExtIndexedOctree >
		: public ExtendedPointTest
		{
			using Point = ExtendedPoint;
			using PointVector = ExtendedPointVector;
			using Octree = ShallowExtIndexedOctree;
			using OctreePtr = ShallowExtIndexedOctreePtr;
			using Test = model::test::OctreeTest< Octree >;
		
		protected:
			/** Creates points that will be inside the octree and the associated expected results of octree construction. */
			void SetUp()
			{
				PointVector points;
				generatePoints( points );
				m_octree = make_shared< Octree >( 1, 10 );
				m_octree->build( points );
			}
			
			OctreePtr m_octree;
		};
		
		template<>
		class OctreeTest< MediumExtIndexedOctree >
		: public ExtendedPointTest
		{
			using Point = ExtendedPoint;
			using PointVector = ExtendedPointVector;
			using Octree = MediumExtIndexedOctree;
			using OctreePtr = MediumExtIndexedOctreePtr;
			using Test = model::test::OctreeTest< Octree >;
		
		protected:
			/** Creates points that will be inside the octree and the associated expected results of octree construction. */
			void SetUp()
			{
				PointVector points;
				generatePoints( points );
				m_octree = make_shared< Octree >( 1, 20 );
				m_octree->build( points );
			}
			
			OctreePtr m_octree;
		};
		
		template<>
		class OctreeTest< ShallowFrontOctree >
		: public SimplePointTest
		{
			using Octree = ShallowFrontOctree;
			using OctreePtr = ShallowFrontOctreePtr;
			using Test = model::test::OctreeTest< Octree >;
		
		protected:
			/** Creates points that will be inside the octree and the associated expected results of octree construction. */
			void SetUp()
			{
				PointVector points;
				generatePoints( points );
				m_octree = make_shared< Octree >( 1, 10 );
				m_octree->build( points );
			}
			
			OctreePtr m_octree;
		};
		
		template<>
		class OctreeTest< MediumFrontOctree >
		: public SimplePointTest
		{
			using Octree = MediumFrontOctree;
			using OctreePtr = MediumFrontOctreePtr;
			using Test = model::test::OctreeTest< Octree >;
		
		protected:
			/** Creates points that will be inside the octree and the associated expected results of octree construction. */
			void SetUp()
			{
				PointVector points;
				generatePoints( points );
				m_octree = make_shared< Octree >( 1, 20 );
				m_octree->build( points );
			}
			
			OctreePtr m_octree;
		};
		
		template<>
		class OctreeTest< ShallowExtFrontOctree >
		: public ExtendedPointTest
		{
			using Point = ExtendedPoint;
			using PointVector = ExtendedPointVector;
			using Octree = ShallowExtFrontOctree;
			using OctreePtr = ShallowExtFrontOctreePtr;
			using Test = model::test::OctreeTest< Octree >;
		
		protected:
			/** Creates points that will be inside the octree and the associated expected results of octree construction. */
			void SetUp()
			{
				PointVector points;
				generatePoints( points );
				m_octree = make_shared< Octree >( 1, 10 );
				m_octree->build( points );
			}
			
			OctreePtr m_octree;
		};
		
		template<>
		class OctreeTest< MediumExtFrontOctree >
		: public ExtendedPointTest
		{
			using Point = ExtendedPoint;
			using PointVector = ExtendedPointVector;
			using Octree = MediumExtFrontOctree;
			using OctreePtr = MediumExtFrontOctreePtr;
			using Test = model::test::OctreeTest< Octree >;
		
		protected:
			/** Creates points that will be inside the octree and the associated expected results of octree construction. */
			void SetUp()
			{
				PointVector points;
				generatePoints( points );
				m_octree = make_shared< Octree >( 1, 20 );
				m_octree->build( points );
			}
			
			OctreePtr m_octree;
		};
		
		template<>
		class OctreeTest< ShallowOutOfCoreOctree >
		: public SimplePointTest
		{
			using Octree = ShallowOutOfCoreOctree;
			using OctreePtr = ShallowOutOfCoreOctreePtr;
			using Test = model::test::OctreeTest< Octree >;
		
		protected:
			/** Creates points that will be inside the octree and the associated expected results of octree construction. */
			void SetUp()
			{
				m_octree = make_shared< Octree >( 1, 10, g_appPath + "/Octree.db" );
				m_octree->buildFromFile( *m_plyFileName, SimplePointReader::SINGLE, Attributes::COLORS );
			}
			
			OctreePtr m_octree;
		};
		
		template<>
		class OctreeTest< MediumOutOfCoreOctree >
		: public SimplePointTest
		{
			using Octree = MediumOutOfCoreOctree;
			using OctreePtr = MediumOutOfCoreOctreePtr;
			using Test = model::test::OctreeTest< Octree >;
		
		protected:
			/** Creates points that will be inside the octree and the associated expected results of octree construction. */
			void SetUp()
			{
				m_octree = make_shared< Octree >( 1, 20, g_appPath + "/Octree.db" );
				m_octree->buildFromFile( *m_plyFileName, SimplePointReader::SINGLE, Attributes::COLORS );
			}
			
			OctreePtr m_octree;
		};
		
		template<>
		class OctreeTest< ShallowExtOutOfCoreOctree >
		: public ExtendedPointTest
		{
			using Point = ExtendedPoint;
			using PointVector = ExtendedPointVector;
			using Octree = ShallowExtOutOfCoreOctree;
			using OctreePtr = ShallowExtOutOfCoreOctreePtr;
			using Test = model::test::OctreeTest< Octree >;
		
		protected:
			/** Creates points that will be inside the octree and the associated expected results of octree construction. */
			void SetUp()
			{
				m_octree = make_shared< Octree >( 1, 10, g_appPath + "/Octree.db" );
				m_octree->buildFromFile( *m_plyFileName, ExtendedPointReader::SINGLE, Attributes::COLORS_AND_NORMALS );
			}
			
			OctreePtr m_octree;
		};
		
		template<>
		class OctreeTest< MediumExtOutOfCoreOctree >
		: public ExtendedPointTest
		{
			using Point = ExtendedPoint;
			using PointVector = ExtendedPointVector;
			using Octree = MediumExtOutOfCoreOctree;
			using OctreePtr = MediumExtOutOfCoreOctreePtr;
			using Test = model::test::OctreeTest< Octree >;
		
		protected:
			/** Creates points that will be inside the octree and the associated expected results of octree construction. */
			void SetUp()
			{
				m_octree = make_shared< Octree >( 1, 20, g_appPath + "/Octree.db" );
				m_octree->buildFromFile( *m_plyFileName, ExtendedPointReader::SINGLE, Attributes::COLORS_AND_NORMALS );
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
		
		void testBoundaries( const ShallowOctree& octree )
		{
			testShallowBoundaries( octree );
		}
		
		void testHierarchy( const ShallowOctree& octree )
		{
			checkHierarchy( octree.getHierarchy() );
		}
		
		void testBoundaries( const MediumOctree& octree )
		{
			testMediumBoundaries( octree );
		}
		
		void testHierarchy( const MediumOctree& octree )
		{
			checkHierarchy( octree.getHierarchy() );
		}
		
		void testBoundaries( const ShallowExtOctree& octree )
		{
			testShallowBoundaries( octree );
		}
		
		void testHierarchy( const ShallowExtOctree& octree )
		{
			checkHierarchy( octree.getHierarchy() );
		}
		
		void testBoundaries( const MediumExtOctree& octree )
		{
			testMediumBoundaries( octree );
		}
		
		void testHierarchy( const MediumExtOctree& octree )
		{
			checkHierarchy( octree.getHierarchy() );
		}
		
		using testing::Types;
		
		typedef Types< 	ShallowOctree, ShallowExtOctree, MediumOctree, MediumExtOctree, ShallowRandomSampleOctree,
						ShallowExtRandomSampleOctree, MediumRandomSampleOctree, MediumExtRandomSampleOctree,
						ShallowIndexedOctree, ShallowExtIndexedOctree, MediumIndexedOctree, MediumExtIndexedOctree,
						ShallowFrontOctree, ShallowExtFrontOctree, MediumFrontOctree, MediumExtFrontOctree,
						ShallowOutOfCoreOctree, ShallowExtOutOfCoreOctree, MediumOutOfCoreOctree,
						MediumExtOutOfCoreOctree > Implementations;
		
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