#ifndef FRUSTUM_H

#include <glm/glm.hpp>

using namespace glm;

namespace model
{
	/** Camera's frustum. Provide culling of axis-aligned boxes. */
	class Frustum
	{
	public:
		Frustum( const mat4x4& modelViewProjection );
		
		/** Updates the frustrum after changing camera's intrinsic parameters ( fov, ratio, near or far plane ). */
		void update( const mat4x4& modelViewProjection );
	
	private:
		// Planes are defined
		vec4[ 6 ] planes;
	};
}

#endif