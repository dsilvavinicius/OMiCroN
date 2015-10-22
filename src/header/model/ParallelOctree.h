#ifndef PARALLEL_OCTREE_H
#define PARALLEL_OCTREE_H

#include <omp.h>
#include "FrontOctree.h"
#include "ParallelFrontBehavior.h"

namespace model
{
	/** An octree that performs front tracking in parallel. */
	template< typename OctreeParameters, typename Front, typename FrontInsertionContainer >
	class ParallelOctree
	: public FrontOctree< OctreeParameters, Front, FrontInsertionContainer >
	{
		using MortonCode = typename OctreeParameters::Morton;
		using MortonVector = vector< MortonCode >;
		using PointVector = IndexVector;
		using PointVectorPtr = shared_ptr< PointVector >;
		using FrontOctree = model::FrontOctree< OctreeParameters, Front, FrontInsertionContainer >;
		using ParallelFrontBehavior = model::ParallelFrontBehavior< OctreeParameters, Front, FrontInsertionContainer >;
	
	public:
		ParallelOctree( const int& maxPointsPerNode, const int& maxLevel );
		
	protected:
		virtual void setupNodeRendering( OctreeNodePtr node, RenderingState& renderingState );
		
	private:
		/** List with the nodes that will be deleted in current front tracking. */
		MortonVector m_frontDeletionList;
	};
	
	template< typename OctreeParameters, typename Front, typename FrontInsertionContainer >
	ParallelOctree< OctreeParameters, Front, FrontInsertionContainer >::ParallelOctree(
		const int& maxPointsPerNode, const int& maxLevel )
	: FrontOctree( maxPointsPerNode, maxLevel )
	{
		FrontOctree::m_frontBehavior = new ParallelFrontBehavior( *this );
	}
	
	template< typename OctreeParameters, typename Front, typename FrontInsertionContainer >
	inline void ParallelOctree< OctreeParameters, Front, FrontInsertionContainer >::setupNodeRendering(
		OctreeNodePtr node, RenderingState& renderingState )
	{
		PointVectorPtr points = node-> template getContents< PointVector >();
		
		#pragma omp critical (FrontRendering)
			renderingState.handleNodeRendering( *points );
	}
}

#endif