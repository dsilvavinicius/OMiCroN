#include "../model/Camera.h"

#include <glm/gtc/epsilon.hpp>
#include <gtest/gtest.h>

using namespace glm;

namespace model
{
	namespace test
	{
        class CameraTest : public ::testing::Test
		{
		protected:
			void SetUp()
			{
				m_camera = make_shared<Camera>();
				m_camera->rotateX(3.1421f * 0.5f);
				m_camera->rotateY(3.1421f * 0.5f);
				m_camera->panX(10.f);
				m_camera->panY(20.f);
			}

			CameraPtr m_camera;
		};

		TEST_F(CameraTest, Creation_And_Operations)
		{
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
