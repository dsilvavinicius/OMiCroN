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

		TEST_F(CameraTest, Creation_And_Operations) {
			vec3::bool_type test = epsilonEqual(m_camera->getForward(), vec3(-1.f, 0.f, 0.f), 1.0e-2f);
			ASSERT_TRUE (test.x && test.y && test.z);
			
			test = epsilonEqual(m_camera->getSide(), vec3(0.f, 1.f, 0.f), 1.0e-2f);
			ASSERT_TRUE (test.x && test.y && test.z);
			
			test = epsilonEqual(m_camera->getUp(), vec3(0.f, 0.f, -1.f), 1.0e-2f);
			ASSERT_TRUE (test.x && test.y && test.z);
			
			test = epsilonEqual(m_camera->getOrigin(), vec3(0.f, 10.f, -20.f), 1.0e-2f);
			ASSERT_TRUE (test.x && test.y && test.z);
		}
	}
}
