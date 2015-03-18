#ifndef FRUSTUM_H
#define FRUSTUM_H

#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <Eigen/Dense>
#include <Eigen/Geometry>
#include "Stream.h"

using namespace std;
using namespace glm;
using namespace Eigen;

namespace model
{
	/** Camera's frustum. Provide culling of axis-aligned boxes. The camera frustum planes will be in model
	 * coordinates. To get the planes in world coordinates, pass a view-projection matrix instead of a
	 * model-view-projection */
	class Frustum
	{
		using Plane = Hyperplane< float, 3 >;
		using Box = AlignedBox< float, 3 >;
	public:
		/** @param mvp is the model-view-projection matrix. */
		Frustum( const Matrix4f& mvp );
		~Frustum();
		
		/** Updates the frustrum after changing camera.
		 *	@param mvp is the model-view-projection matrix. */
		void update( const Matrix4f& mvp );
		
		/** Performs optimized view frustum culling as explained in paper Optimized View Frustum Culling Algorithms for
		 * Bounding Boxes. Available in: @link http://www.cse.chalmers.se/~uffe/vfc_bbox.pdf. Differently from the algorithm
		 * there, we don't distinguish intersection and outside cases.
		 * @returns true if the box is outside the frustum, false otherwise. */
		bool isCullable( const Box& box );
		
		friend ostream& operator<<( ostream& out, const Frustum& f );
		
	private:
		/** Algorithm for extraction of the 6 frustum planes from the model-view-projection matrix as explained in paper
		 * Fast Extraction of Viewing Frustum Planes from the World-View-Projection Matrix. Available in
		 * @link gamedevs.org/uploads/fast-extraction-viewing-frustum-planes-from-world-view-projection-matrix.pdf .
		 * @param normalize indicates that the final plane equation should be normalized. */
		void extractPlanes( const Matrix4f& mvp, const bool& normalize );
		
		/** Frustum planes. */
		vector< Plane* > m_planes;
	};
	
	inline bool Frustum::isCullable( const Box& box )
	{
		//cout << "==== Starting culling ====" << endl << "Box: " << box << endl;
		
		Vector3f boxSizes = box.sizes();
		Vector3f boxMin = box.min();
		Vector3f boxMax = box.max();
		
		// Performs frustum plane dilatation, finds n only, since p is necessary just for detecting intersections points,
		// and verifies if n is inside or outside the frustum.
		for( int i = 0; i < 6; ++i )
		{
			Plane* plane = m_planes[ i ];
			Vector3f normal = plane->normal();
			float dilatation = abs( boxSizes.dot( normal ) );
			float offset = plane->offset() - dilatation;
			Plane dilatatedPlane( normal, offset );
			
			//cout << endl << "Dilatation:" << dilatation << endl << "Dilatated plane:" << dilatatedPlane << endl;
			
			Vector3f n;
			
			n[ 0 ] = ( normal[ 0 ] < 0 ) ? boxMax[ 0 ] : boxMin[ 0 ];
			n[ 1 ] = ( normal[ 1 ] < 0 ) ? boxMax[ 1 ] : boxMin[ 1 ];
			n[ 2 ] = ( normal[ 2 ] < 0 ) ? boxMax[ 2 ] : boxMin[ 2 ];
			
			float signedDist = dilatatedPlane.signedDistance( n );
			//cout << "n point: " << n << endl << "signedDist:" << signedDist << endl;
			
			if( signedDist > 0 )
			{
				/*string planeName;
				switch( i )
				{
					case 0: planeName = "left"; break;
					case 1: planeName = "right"; break;
					case 2: planeName = "top"; break;
					case 3: planeName = "bot"; break;
					case 4: planeName = "near"; break;
					case 5: planeName = "far"; break;
				}
				cout << "Outside " << planeName << " plane." << endl;*/
				return true;
			}
		}
		return false;
	}
}

#endif