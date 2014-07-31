#include "../model/MortonCode.h"

#include <glm/gtc/epsilon.hpp>
#include <gtest/gtest.h>

using namespace glm;

namespace model
{
	namespace test
	{
        class MortonCodeTest : public ::testing::Test
		{
		protected:
			void SetUp()
			{
				ShallowMortonCode morton();
			}

			CameraPtr m_mortonCode;
		};

		TEST_F(MortonCodeTest, Creation) {
			
		}
		
		TEST_F(MortonCodeTest, Unicity) {
			
		}
	}
}
