#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

using namespace glm;

namespace model
{
	/** Free camera. Specified as the OpenGL camera, i.e. as the coordinate system
	* with axis -x, y and -z, with x, y and z being the default global frame. */
	class Camera
	{
	public:
		/** Creates the camera, given camera origin, and original*/
		Camera(vec3 origin, const quat& rotation = quat(1.f, 0.f, 0.f, 0.f));

		/** Rotates around local X (radians). */
		void rotateX(float angle);
		/** Rotates around local Y (radians). */
		void rotateY(float angle);
		/** Pans on local X axis. */
		void panX(float displacement);
		/** Pans on local Y axis. */
		void panY(float displacement);
		/** Zooms in the direction of the forward axis. */
		void zoom(float displacement);

	private:
		/** Camera position. */
		vec3 m_origin;
		/** Forward vector (-z).*/
		vec3 m_forward;
		/** Side vector (-x). */
		vec3 m_side;
		/** Up vector (y). */
		vec3 m_up;
		/** Quaternion that specifies current camera rotation. */
		quat m_rotation;
	};
}

#endif // CAMERA_H