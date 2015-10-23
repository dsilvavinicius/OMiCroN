#ifndef PARALLEL_OCTREE_H
#define PARALLEL_OCTREE_H

#include <omp.h>
#include "FrontOctree.h"
#include "ParallelFrontBehavior.h"

namespace model
{
	/** An octree that performs front tracking in parallel. */
	template< typename OctreeParams, typename Front, typename FrontInsertionContainer >
	class ParallelOctree
	: public FrontOctree< OctreeParams, Front, FrontInsertionContainer >
	{
		using MortonCode = typename OctreeParams::Morton;
		using MortonVector = vector< MortonCode >;
		using PointVector = IndexVector;
		using PointVectorPtr = shared_ptr< PointVector >;
		using FrontOctree = model::FrontOctree< OctreeParams, Front, FrontInsertionContainer >;
		using ParallelFrontBehavior = model::ParallelFrontBehavior< OctreeParams, Front, FrontInsertionContainer >;
	
	public:
		ParallelOctree( const int& maxPointsPerNode, const int& maxLevel );
		
	protected:
		virtual void setupNodeRendering( OctreeNodePtr node, RenderingState& renderingState );
		
	private:
		/** List with the nodes that will be deleted in current front tracking. */
		MortonVector m_frontDeletionList;
	};
	
	template< typename OctreeParams, typename Front, typename FrontInsertionContainer >
	ParallelOctree< OctreeParams, Front, FrontInsertionContainer >::ParallelOctree(
		const int& maxPointsPerNode, const int& maxLevel )
	: FrontOctree( maxPointsPerNode, maxLevel )
	{
		FrontOctree::m_frontBehavior = new ParallelFrontBehavior( *this );
	}
	
	template< typename OctreeParams, typename Front, typename FrontInsertionContainer >
	inline void ParallelOctree< OctreeParams, Front, FrontInsertionContainer >::setupNodeRendering(
		OctreeNodePtr node, RenderingState& renderingState )
	{
		PointVectorPtr points = node-> template getContents< PointVector >();
		
		#pragma omp critical (FrontRendering)
			renderingState.handleNodeRendering( *points );
	}
}

#endif