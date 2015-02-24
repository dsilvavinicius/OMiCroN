#ifndef FRUSTUM_H
#define FRUSTUM_H

#include <glm/glm.hpp>
#include <Eigen/Dense>
#include <Eigen/Geometry>

using namespace glm;
using namespace Eigen;

namespace model
{
	/** Camera's frustum. Provide culling of axis-aligned boxes. */
	class Frustum
	{
	public:
		Frustum( const Matrix4f& modelViewProjection );
		
		/** Updates the frustrum after changing camera's intrinsic parameters ( fov, ratio, near or far plane ). */
		void update( const Matrix4f& modelViewProjection );
	
		
		
	private:
		/** Extracts the 6 frustum planes from the model-view-projection matrix.
		 * @param normalize indicates that the final plane equation should be normalized. */
		void extractPlanes( const Matrix4f& modelViewProjection, const bool& normalize );
		
		// Planes are defined
		Hyperplane< float, 3 >* m_planes[ 6 ];
	};
}

#endif