#ifndef POINT_CLUSTERING_H
#define POINT_CLUSTERING_H
#include "Array.h"

namespace omicron
{
	/** Point set reduction based on normal deviation clustering as described in the paper High Quality Interactive
	 * Rendering of Massive Point Models using Multi-way kd-Trees
	 * ( http://www.crs4.it/vic/data/papers/pg2010-multi-way-kdtrees.pdf ). */
	template< typename Point >
	class PointSetReduction
	{
	public:
		using PointArray = Array< Point >;
		
		/**
		 * Ctor.
		 * @param normalThresh is the normal deviation threshold used to cluster points.
		 */
		PointSetReduction( const float normalThresh );
	
		/**
		 * Reduce the given point set, resulting in a representative set with less points. 
		 * @param size is the size of the resulting reduced point set.
		 */
		PointArray reduce( const PointArray& points, const ulong size );
	
	private:
		ulong m_pointsPerNode;
		float m_normalThresh;
	};
	
	template< typename Point >
	PointSetReduction< Point >::PointSetReduction( const float normalThresh )
	{
	}
	
	template< typename Point >
	typename PointSetReduction< Point >::PointArray PointSetReduction< Point >
	::reduce( const PointArray& points, const ulong size )
	{
		// 1) Quantize points in a K³ grid
		// 2) Make a per-cell cluster list, which each point being a cluster
		// 3) Push all cluster lists into a priority queue prioritized by list size
		//
		// 4) Until the number of points is reached:
		// 5) 	Merge clusters if the normal deviation is less than threshold
		//
		// 6) For each cluster
		// 7) 	Create a representative point which is a size-weighted average of all points in the cluster
		//
		// Notes: K should be wisely choosen. The normal threshold must be relaxed if the number of clusters is greater
		// than the expected final size.
	}
}

#endif
