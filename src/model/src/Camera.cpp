#include "model/header/Camera.h"

namespace model
{
	Camera::Camera(vec3 origin, const quat& rotation)
	{
		m_origin = origin;
		m_rotation = rotation;
		m_forward = m_rotation * vec3(0.f, 0.f, -1.f);
		m_side = m_rotation * vec3(-1.f, 0.f, 0.f);
		m_up = m_rotation * vec3(0.f, 1.f, 0.f);
	}

	void Camera::rotateX(float angle)
	{
		rotate(m_rotation, 3.1421f * 0.1f, m_side);
	}

	void Camera::rotateY(float angle)
	{
		rotate(m_rotation, 3.1421f * 0.1f, m_up);
	}

	void Camera::panX(float displacement)
	{
		m_origin += m_side * displacement;
	}

	void Camera::panY(float displacement)
	{
		m_origin += m_up * displacement;
	}

	void Camera::zoom(float displacement)
	{
		m_origin += m_forward * displacement;
	}
}