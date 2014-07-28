#include "Octree.h"
#include <limits>

namespace model
{
	template <typename MortonPrecision, typename VecType>
	Octree::Octree()
	{
		m_size = make_shared<VecType>();
	}
	
	template <typename MortonPrecision, typename VecType>
	void Octree<MortonPrecision>::build(vector<shared_ptr<Point>> points)
	{
		buildBoundaries(points);
	}
	
	template <typename MortonPrecision, typename VecType>
	void Octree<MortonPrecision>::buildBoundaries(vector<shared_ptr<Point>> points)
	{
		double negInf = numeric_limits<double>::min();
		double posInf = numeric_limits<double>::max();
		VecType minCoords();
		VecType maxCoords();
		
		for (Point point : points)
		{
			
		}
	}
	
	template <typename MortonPrecision, typename VecType>
	void Octree<MortonPrecision>::traverse()
	{
		
	}
}