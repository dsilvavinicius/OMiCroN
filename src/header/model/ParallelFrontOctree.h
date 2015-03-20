#ifndef PARALLEL_FRONT_OCTREE_H

#include <omp.h>
#include "FrontOctree.h"

namespace model
{
	/** An octree that performs front tracking in parallel. */
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point, typename Front >
	class ParallelFrontOctree
	: FrontOctree< MortonPrecision, Float, Vec3, Point, Front >
	{
		using MortonCode = model::MortonCode< MortonPrecision >;
		using MortonVector = vector< MortonCode >;
		
	private:
		/** List with the nodes that will be deleted in current front tracking. */
		MortonVector m_frontDeletionList;
	};
}

#endif