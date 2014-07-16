#include "Camera.h"

#include <iostream>

using namespace std;

namespace model
{
	vec3 Camera::xAxis(-1.f, 0.f, 0.f);
	vec3 Camera::yAxis(0.f, 1.f, 0.f);
	vec3 Camera::zAxis(0.f, 0.f, -1.f);
	
	Camera::Camera(const vec3& origin, const quat& rotation)
	{
		m_origin = origin;
		m_rotation = rotation;
		applyCurrentRotation();
	}

	void Camera::rotateX(const float& angle)
	{
		cout << "rotateX" << endl;
		rotate(angle, m_side);
	}

	void Camera::rotateY(const float& angle)
	{
		cout << "rotateY" << endl;
		rotate(angle, m_up);
	}
	
	void Camera::rotate(const float& angle, const vec3& axis)
	{
		m_rotation = angleAxis(angle, axis) * m_rotation;
		applyCurrentRotation();
		
		cout << "forward {" 
		<< m_forward.x << " " << m_forward.y << " " << m_forward.z 
		<< "}" << endl;
		  
		cout << "side {" 
		<< m_side.x << " " << m_side.y << " " << m_side.z 
		<< "}" << endl;
		
		cout << "up {" 
		<< m_up.x << " " << m_up.y << " " << m_up.z 
		<< "}" << endl;
	}
	
	void Camera::applyCurrentRotation()
	{
		m_forward = m_rotation * zAxis;
		m_side = m_rotation * xAxis;
		m_up = m_rotation * yAxis;
	}

	void Camera::panX(const float& displacement)
	{
		m_origin += m_side * displacement;
	}

	void Camera::panY(const float& displacement)
	{
		m_origin += m_up * displacement;
	}

	void Camera::zoom(const float& displacement)
	{
		m_origin += m_forward * displacement;
	}
	
	vec3& Camera::getOrigin() { return m_origin; }
	vec3& Camera::getForward() { return m_forward; }
	vec3& Camera::getSide() { return m_side; }
	vec3& Camera::getUp() { return m_up; }
	quat& Camera::getRotation() { return m_rotation; }
}