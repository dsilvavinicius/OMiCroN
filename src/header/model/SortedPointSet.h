#ifndef SORTED_POINT_SET_H
#define SORTED_POINT_SET_H

#include "OctreeDimensions.h"

namespace model
{
	template< typename Morton >
	struct SortedPointSet
	{
		using PointVector = vector< Point, TbbAllocator< Point > >;
		using PointVectorPtr = shared_ptr< PointVector >;
		using OctreeDim = OctreeDimensions< Morton >;
		
		SortedPointSet() {}
		
		SortedPointSet( PointVectorPtr points, const OctreeDim& dim )
		: m_points( points ), m_dim( dim )
		{}
		
		PointVectorPtr m_points;
		OctreeDim m_dim;
	};
}

#endif