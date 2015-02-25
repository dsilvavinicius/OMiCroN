#ifndef FRUSTUM_H
#define FRUSTUM_H

#include <vector>
#include <glm/glm.hpp>
#include <Eigen/Dense>
#include <Eigen/Geometry>

using namespace std;
using namespace glm;
using namespace Eigen;

namespace model
{
	/** Camera's frustum. Provide culling of axis-aligned boxes. */
	class Frustum
	{
		using Plane = Hyperplane< float, 3 >;
		using Box = AlignedBox< float, 3 >;
	public:
		Frustum( const Matrix4f& modelViewProjection );
		~Frustum();
		
		/** Updates the frustrum after changing camera's intrinsic parameters ( fov, ratio, near or far plane ). */
		void update( const Matrix4f& modelViewProjection );
		
		/** Performs optimized view frustum culling as explained in paper Optimized View Frustum Culling Algorithms for
		 * Bounding Boxes. Available in: @link http://www.cse.chalmers.se/~uffe/vfc_bbox.pdf. Differently from the algorithm
		 * there, we don't distinguish intersection and outside cases.
		 * @returns true if the box is outside the frustum, false otherwise. */
		inline bool isCullable( const Box& box );
		
	private:
		/** Algorithm for extraction of the 6 frustum planes from the model-view-projection matrix as explained in paper
		 * Fast Extraction of Viewing Frustum Planes from the World-View-Projection Matrix. Available in
		 * @link gamedevs.org/uploads/fast-extraction-viewing-frustum-planes-from-world-view-projection-matrix.pdf
		 * @param normalize indicates that the final plane equation should be normalized. */
		void extractPlanes( const Matrix4f& modelViewProjection, const bool& normalize );
		
		// Planes are defined
		vector< Plane* > m_planes;
	};
}

#endif