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
		buildNodes(points);
	}
	
	template <typename MortonPrecision, typename Float, typename Vec3>
	void Octree<MortonPrecision, Float, Vec3>::buildBoundaries(
		vector< PointPtr<Vec3> > points)
	{
		Float negInf = numeric_limits<Float>::min();
		Float posInf = numeric_limits<Float>::max();
		Vec3 minCoords(posInf, posInf, posInf);
		Vec3 maxCoords(negInf, negInf, negInf);
		
		for (PointPtr<Vec3> point : points)
		{
			Vec3 pos = point->getPos();
			
			for (int i = 0; i < 3; ++i)
			{
				minCoords[i] = min(minCoords[i], pos[i]);
				maxCoords[i] = max(maxCoords[i], pos[i]);
			}
		}
		
		*m_origin = minCoords;
		*m_size = maxCoords - minCoords;
	}
	
	template <typename MortonPrecision, typename Float, typename Vec3>
	void Octree< MortonPrecision, Float, Vec3 >::buildNodes(
		vector< PointPtr< Vec3 > > points)
	{
		
	}
	
	template <typename MortonPrecision, typename Float, typename Vec3>
	void Octree<MortonPrecision, Float, Vec3>::traverse()
	{
		
	}
}