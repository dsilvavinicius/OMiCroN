#include <gtest/gtest.h>
#include <iostream>
#include "Front.h"
#include "MortonCode.h"

using namespace std;

namespace model
{
	namespace test
	{
        class FrontTest : public ::testing::Test
		{
		protected:
			void SetUp()
			{
				setlocale( LC_NUMERIC, "C" );
			}
		};
		
		TEST_F( FrontTest, ReproduceDuplicatePlaceholderInsertion )
		{
// 			using Morton = MediumMortonCode;
// 			using Front = model::Front< Morton, Point >;
// 			using OctreeDim = typename Front::OctreeDim;
// 			
// 			OctreeDim dim( Vec3( 0.f, 0.f, 0.f ), Vec3( 0.3994904160499573f, 0.2322048395872116f, 1.0f), 13 );
// 			
// 			Front front( "don't care", dim, 4, 1024ul * 1024ul * 1024ul * 1 );
// 			
// 			Morton morton0; morton0.build( 0xa46c741000ul );
// 			front.insertPlaceholder( morton0, 0 );
// 			
// 			Morton morton1; morton1.build( 0xce57c3d300ul );
// 			front.insertPlaceholder( morton1, 0 );
// 			
// 			Morton morton2; morton2.build( 0x956ac16f00ul );
// 			front.insertPlaceholder( morton2, 0 );
		}
	}
}