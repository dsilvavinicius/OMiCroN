#ifndef SORTED_POINT_SET_H
#define SORTED_POINT_SET_H

#include "OctreeDimensions.h"
#include <deque>

namespace model
{
	/** A point set with the points and dimensional grid data. */
	template< typename Morton >
	struct PointSet
	{
		using PointDeque = deque< Point, TbbAllocator< Point > >;
		using PointDequePtr = shared_ptr< PointDeque >;
		using OctreeDim = OctreeDimensions< Morton >;
		
		PointSet() {}
		
		PointSet( PointDequePtr points, const OctreeDim& dim )
		: m_points( points ), m_dim( dim )
		{}
		
		PointDequePtr m_points;
		OctreeDim m_dim;
	};
}

#endif