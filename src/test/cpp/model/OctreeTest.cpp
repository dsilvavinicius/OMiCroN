#include "Octree.h"

#include <gtest/gtest.h>

namespace model
{
	namespace test
	{
        class OctreeTest : public ::testing::Test
		{
		protected:
			/** Creates points that will be inside the octree and the associated expected results of octree construction. */
			void SetUp()
			{
				// These points should define the boundaries of the octree hexahedron.
				auto up = make_shared< Point< float, vec3 > >(vec3(0.01f, 0.02f, 0.03f), vec3(1.f, 15.f ,2.f));
				auto down = make_shared< Point< float, vec3 > >(vec3(0.04f, 0.05f, 0.06f), vec3(3.f, -31.f ,4.f));
				auto left = make_shared< Point< float, vec3 > >(vec3(0.07f, 0.08f, 0.09f), vec3(-14.f, 5.f ,6.f));
				auto right = make_shared< Point< float, vec3 > >(vec3(0.1f, 0.11f, 0.12f), vec3(46.f, 7.f ,8.f));
				auto front = make_shared< Point< float, vec3 > >(vec3(0.13f, 0.14f, 0.15f), vec3(9.f, 10.f ,24.f));
				auto back = make_shared< Point< float, vec3 > >(vec3(0.16f, 0.17f, 0.18f), vec3(11.f, 12.f ,-51.f));
				
				// Additional points inside the hexahedron.
				auto addPoint0 = make_shared< Point< float, vec3 > >(vec3(0.19f, 0.2f, 0.21f), vec3(13.f, -12.f, 9.f));
				auto addPoint1 = make_shared< Point< float, vec3 > >(vec3(0.22f, 0.23f, 0.24f), vec3(-5.f, -8.f, 1.f));
				auto addPoint2 = make_shared< Point< float, vec3 > >(vec3(0.25f, 0.26f, 0.27f), vec3(14.f, 11.f, -4.f));
				auto addPoint3 = make_shared< Point< float, vec3 > >(vec3(0.28f, 0.29f, 0.30f), vec3(7.f, 3.f, -12.f));
				auto addPoint4 = make_shared< Point< float, vec3 > >(vec3(0.31f, 0.32f, 0.33f), vec3(12.f, 5.f, 0.f));
				
				m_points.push_back(back);
				m_points.push_back(front);
				m_points.push_back(right);
				m_points.push_back(left);
				m_points.push_back(down);
				m_points.push_back(up);
				
				m_points.push_back(addPoint0);
				m_points.push_back(addPoint1);
				m_points.push_back(addPoint2);
				m_points.push_back(addPoint3);
				m_points.push_back(addPoint4);
			}
			
			PointVector< float, vec3 > m_points;
		};

		/** Tests the calculated boundaries of the ShallowOctree. */
		TEST_F(OctreeTest, ShallowBoundaries)
		{
			auto octree = make_shared< ShallowOctree<float, vec3> >(1);
			octree->build(m_points);
			
			ASSERT_EQ(octree->getMaxLevel(), 10);
			ASSERT_EQ(octree->getMaxPointsPerNode(), 1);
			
			vec3 origin = *octree->getOrigin();
			vec3 size = *octree->getSize();
			vec3 leafSize = *octree->getLeafSize();
			
			float epsilon = 10.e-8f;
			ASSERT_TRUE( distance2( origin, vec3( -14.f, -31.f, -51.f ) ) < epsilon ) ;
			ASSERT_TRUE( distance2( size, vec3(60.f, 46.f, 75.f) ) < epsilon );
			ASSERT_TRUE( distance2( leafSize, vec3(0.05859375f, 0.044921875f, 0.073242188f ) ) < epsilon ) ;
		}
		
		template<typename MortonPrecision, typename Float, typename Vec3>
		void checkNode(OctreeMapPtr< MortonPrecision, Float, Vec3 > hierarchy, const MortonPrecision& bits)
		{
			stringstream ss;
			ss << hex << bits << dec;
			SCOPED_TRACE( ss.str() );
			auto code = make_shared< MortonCode< MortonPrecision > >();
			code->build( bits );
			auto iter = hierarchy->find(code);
			ASSERT_FALSE( iter == hierarchy->end() );
			hierarchy->erase( iter );
		}
		
		template< typename Float, typename Vec3 >
		void checkSparseHierarchy(ShallowOctreeMapPtr< Float, Vec3 > hierarchy)
		{
			
			checkNode< unsigned int, Float, Vec3 >(hierarchy, 0xa6c3u);
			checkNode< unsigned int, Float, Vec3 >(hierarchy, 0xa6c0u);
			checkNode< unsigned int, Float, Vec3 >(hierarchy, 0xc325u);
			checkNode< unsigned int, Float, Vec3 >(hierarchy, 0xc320u);
			checkNode< unsigned int, Float, Vec3 >(hierarchy, 0x1d82u);
			checkNode< unsigned int, Float, Vec3 >(hierarchy, 0x1d80u);
			checkNode< unsigned int, Float, Vec3 >(hierarchy, 0x39fu);
			checkNode< unsigned int, Float, Vec3 >(hierarchy, 0x39du);
			checkNode< unsigned int, Float, Vec3 >(hierarchy, 0x67u);
			checkNode< unsigned int, Float, Vec3 >(hierarchy, 0x61u);
			checkNode< unsigned int, Float, Vec3 >(hierarchy, 0x70u);
			checkNode< unsigned int, Float, Vec3 >(hierarchy, 0x71u);
			checkNode< unsigned int, Float, Vec3 >(hierarchy, 0x73u);
			checkNode< unsigned int, Float, Vec3 >(hierarchy, 0x76u);
			checkNode< unsigned int, Float, Vec3 >(hierarchy, 0xau);
			checkNode< unsigned int, Float, Vec3 >(hierarchy, 0xcu);
			checkNode< unsigned int, Float, Vec3 >(hierarchy, 0xeu);
			checkNode< unsigned int, Float, Vec3 >(hierarchy, 0x1u);
		}
		
		/** Tests the ShallowOctree created hierarchy. */
		TEST_F(OctreeTest, ShallowHierarchy)
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
			
			auto octree = make_shared< ShallowOctree<float, vec3> >(1);
			octree->build(m_points);
			
			ShallowOctreeMapPtr< float, vec3 > hierarchy = octree->getHierarchy();
			// Check node that should appear in the sparse representation of the octree.
			checkSparseHierarchy< float, vec3 >(hierarchy);
			
			// Check the other nodes, which would be merged in a sparse octree.
			checkNode< unsigned int, float, vec3 >(hierarchy, 0x14d8);
			checkNode< unsigned int, float, vec3 >(hierarchy, 0x1864);
			checkNode< unsigned int, float, vec3 >(hierarchy, 0x29b);
			checkNode< unsigned int, float, vec3 >(hierarchy, 0x30c);
			checkNode< unsigned int, float, vec3 >(hierarchy, 0x3b0);
			checkNode< unsigned int, float, vec3 >(hierarchy, 0x53);
			
			ASSERT_TRUE(hierarchy->empty());
		}
		
		// TODO: Activate this test when the system has implemented sparse octree representation.
		/** Checks generated sparse octree. */
		/*TEST_F(OctreeTest, ShallowSparse_Hierarchy)
		{	
			// Expected hierarchy. 0x1 is the root node. The blank spaces are merged nodes. A node with an arrow that
			// points to nothing means that it is a sibling of the node at the same position at the line immediately
			// above.
			//
			// 0xa6c3 -> 	______ -> _____ -> ____ -> 0xa -> 0x1
			// 0xa6c0 ->
			// 								   0x67 -> 0xc ->
			// 0xc325 -> 	______ -> _____ -> 0x61 ->
			// 0xc320 ->
			// 				______ -> _____ -> 0x70 -> 0xe ->
			// 				______ -> _____ -> 0x71 ->
			// 				______ -> 0x39f -> 0x73 ->
			// 				______ -> 0x39d -> 
			// 				0x1d82 -> _____ -> 0x76 ->
			// 				0x1d80 ->
			
			auto octree = make_shared< ShallowOctree<float, vec3> >(1);
			octree->build(m_points);
			
			ShallowOctreeMapPtr< float, vec3 > hierarchy = octree->getHierarchy();
			checkSparseHierarchy< float, vec3 >(hierarchy);
			
			ASSERT_TRUE(hierarchy->empty());
		}*/
		
		/** Tests the calculated boundaries of the MediumOctree. */
		TEST_F( OctreeTest, MediumBoundaries )
		{
			auto octree = make_shared< MediumOctree<float, vec3> >( 1 );
			octree->build( m_points );
			
			ASSERT_EQ(octree->getMaxLevel(), 21);
			ASSERT_EQ(octree->getMaxPointsPerNode(), 1);
			
			vec3 origin = *octree->getOrigin();
			vec3 size = *octree->getSize();
			vec3 leafSize = *octree->getLeafSize();
			
			float epsilon = 10.e-8f;
			ASSERT_TRUE( distance2( origin, vec3( -14.f, -31.f, -51.f ) )  < epsilon );
			ASSERT_TRUE( distance2( size, vec3( 60.f, 46.f, 75.f ) ) < epsilon );
			ASSERT_TRUE( distance2( leafSize, vec3( 0.00002861f, 0.000021935f, 0.000035763f ) ) < epsilon );
		}
		
		/** Tests the MediumOctree created hierarchy. */
		TEST_F( OctreeTest, MediumHierarchy )
		{
/*
Expected hierarchy. 0x1 is the root node. A node with an arrow that points to nothing means that
it is a sibling of the node at the same position at the line immediately above.
									   0x1924924924920 -> 0x324924924924 -> 0x64924924924 -> 0xc924924924 -> 0x1924924924 -> 0x324924924 -> 0x64924924 -> 0xc924924 -> 0x1924924 -> 0x324924 -> 0x64924 -> 0xc924 -> 0x1924 -> 0x324 -> 0x64 -> 0xc -> 0x1
0x64924924924927 -> 0xc924924924924 -> 0x1924924924924 ->
0x64924924924920 ->
0x40000000000002 -> 0x8000000000000 -> 0x1000000000000 -> 0x200000000000 -> 0x40000000000 -> 0x8000000000 -> 0x1000000000 -> 0x200000000 -> 0x40000000 -> 0x8000000 -> 0x1000000 -> 0x200000 -> 0x40000 -> 0x8000 -> 0x1000 -> 0x200 -> 0x40 -> 0x8 ->
0x40000000000001 -> 
					0x8000000000004 ->
									   0x1000000000001 ->
																																																												0x9 ->
					0xa492492492490 -> 0x1492492492492 -> 0x292492492492 -> 0x52492492492 -> 0xa492492492 -> 0x1492492492 -> 0x292492492 -> 0x52492492 -> 0xa492492 -> 0x1492492 -> 0x292492 -> 0x52492 -> 0xa492 -> 0x1492 -> 0x292 -> 0x52 -> 0xa ->
					0xa492492492492 ->
																																																												0xb ->
*/
			
			/*for( PointPtr< float, vec3 > point : m_points )
			{
				vec3 pos = *( point->getPos() );
				MediumMortonCode code;
				code.build( pos.x, pos.y, pos.z, 21 );
				code.printPathToRoot( cout, true );
			}*/
			
			auto octree = make_shared< MediumOctree<float, vec3> >( 1 );
			octree->build( m_points );
			MediumOctreeMapPtr< float, vec3 > hierarchy = octree->getHierarchy();
			
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x40000000000001ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x40000000000002ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x64924924924920ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x64924924924927ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0xa492492492492ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0xa492492492490ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x8000000000004ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x8000000000000ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0xc924924924924ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x1492492492492ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x1000000000001ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x1000000000000ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x1924924924924ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x1924924924920ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x292492492492ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x200000000000ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x324924924924ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x52492492492ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x40000000000ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x64924924924ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0xa492492492ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x8000000000ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0xc924924924ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x1492492492ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x1000000000ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x1924924924ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x292492492ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x200000000ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x324924924ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x52492492ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x40000000ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x64924924ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0xa492492ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x8000000ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0xc924924ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x1492492ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x1000000ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x1924924ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x292492ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x200000ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x324924ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x52492ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x40000ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x64924ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0xa492ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x8000ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0xc924ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x1492ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x1000ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x1924ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x292ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x200ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x324ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x52ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x40ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x64ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0xbul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0xaul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x9ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x8ul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0xcul );
			checkNode< unsigned long, float, vec3 >( hierarchy, 0x1ul );
		}
	}
}