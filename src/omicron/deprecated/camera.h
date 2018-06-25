// #ifndef CAMERA_H
// #define CAMERA_H
// 
// #include <memory>
// 
// #define GLM_FORCE_RADIANS
// #include <glm/glm.hpp>
// #include <glm/gtc/quaternion.hpp>
// #undef GLM_FORCE_RADIANS
// 
// using namespace glm;
// using namespace std;
// 
// namespace omicron
// {
// 	/** Free camera. Specified as the OpenGL camera, i.e. as the coordinate system
// 	* with axis -x, y and -z, with x, y and z being the default global frame. */
// 	class Camera
// 	{
// 	public:
// 		/** Creates the camera, given camera origin, and original*/
// 		Camera(const vec3& origin = vec3(0.f, 0.f, 0.f), const quat& rotation = quat(1.f, 0.f, 0.f, 0.f));
// 
// 		/** Rotates around local X (radians). */
// 		void rotateX(const float& angle);
// 		/** Rotates around local Y (radians). */
// 		void rotateY(const float& angle);
// 		/** Pans on local X axis. */
// 		void panX(const float& displacement);
// 		/** Pans on local Y axis. */
// 		void panY(const float& displacement);
// 		/** Zooms in the direction of the forward axis. */
// 		void zoom(const float& displacement);
// 		
// 		vec3& getOrigin();
// 		vec3& getForward();
// 		vec3& getSide();
// 		vec3& getUp();
// 		quat& getRotation();
// 
// 	private:
// 		/** Rotates the current camera frame around the given axis. */
// 		void rotate(const float& angle, const vec3& axis);
// 		
// 		/** Applies current quaternion rotation. */
// 		void applyCurrentRotation();
// 		
// 		/** Camera position. */
// 		vec3 m_origin;
// 		/** Forward vector (-z).*/
// 		vec3 m_forward;
// 		/** Side vector (-x). */
// 		vec3 m_side;
// 		/** Up vector (y). */
// 		vec3 m_up;
// 		/** Quaternion that specifies current camera rotation. */
// 		quat m_rotation;
// 		
// 		// Camera default axis constants in world coordinates.
// 		static vec3 xAxis;
// 		static vec3 yAxis;
// 		static vec3 zAxis;
// 	};
// 	
// 	using CameraPtr = shared_ptr<Camera>;
// }
// 
// #endif // CAMERA_H
