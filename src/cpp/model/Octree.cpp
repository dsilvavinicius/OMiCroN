#include "Octree.h"
#include <limits>

namespace model
{
	template <typename MortonPrecision, typename Float, typename Vec3>
	Octree<MortonPrecision, Float, Vec3>::Octree()
	{
		m_size = make_shared<Vec3>();
	}
	
	template <typename MortonPrecision, typename Float, typename Vec3>
	void Octree<MortonPrecision, Float, Vec3>::build(vector< PointPtr<Vec3> > points)
	{
		buildBoundaries(points);
	}
	
	template <typename MortonPrecision, typename Float, typename Vec3>
	void Octree<MortonPrecision, Float, Vec3>::buildBoundaries(
		vector< PointPtr<Vec3> > points)
	{
		Float negInf = numeric_limits<Float>::min();
		Float posInf = numeric_limits<Float>::max();
		Vec3 minCoords(negInf, negInf, negInf);
		Vec3 maxCoords(posInf, posInf, posInf);
		
		for (PointPtr<Vec3> point : points)
		{
			
		}
	}
	
	template <typename MortonPrecision, typename Float, typename Vec3>
	void Octree<MortonPrecision, Float, Vec3>::traverse()
	{
		
	}
}