#ifndef PARALLEL_OCTREE_H
#define PARALLEL_OCTREE_H

#include <omp.h>
#include "FrontOctree.h"
#include "ParallelFrontBehavior.h"

namespace model
{
	/** An octree that performs front tracking in parallel. */
	template< typename MortonCode, typename Point, typename Front,
			  typename FrontInsertionContainer >
	class ParallelOctree
	: public FrontOctree< MortonCode, Point, Front, FrontInsertionContainer >
	{
		using MortonVector = vector< MortonCode >;
		using PointVector = vector< unsigned int >;
		using PointVectorPtr = shared_ptr< PointVector >;
		using OctreeNodePtr = model::OctreeNodePtr< MortonCode >;
		using FrontOctree = model::FrontOctree< MortonCode, Point, Front, FrontInsertionContainer >;
		using ParallelFrontBehavior = model::ParallelFrontBehavior< MortonCode, Point, Front, FrontInsertionContainer >;
	
	public:
		ParallelOctree( const int& maxPointsPerNode, const int& maxLevel );
		
	protected:
		virtual void setupNodeRendering( OctreeNodePtr node, RenderingState& renderingState );
		
	private:
		/** List with the nodes that will be deleted in current front tracking. */
		MortonVector m_frontDeletionList;
	};
	
	template< typename MortonCode, typename Point, typename Front,
			  typename FrontInsertionContainer >
	ParallelOctree< MortonCode, Point, Front, FrontInsertionContainer >::ParallelOctree(
		const int& maxPointsPerNode, const int& maxLevel )
	: FrontOctree( maxPointsPerNode, maxLevel )
	{
		FrontOctree::m_frontBehavior = new ParallelFrontBehavior( *this );
	}
	
	template< typename MortonCode, typename Point, typename Front,
			  typename FrontInsertionContainer >
	inline void ParallelOctree< MortonCode, Point, Front, FrontInsertionContainer >::setupNodeRendering(
		OctreeNodePtr node, RenderingState& renderingState )
	{
		PointVectorPtr points = node-> template getContents< PointVector >();
		
		#pragma omp critical (FrontRendering)
			renderingState.handleNodeRendering( *points );
	}
	
	//=====================================================================
	// Type Sugar.
	//=====================================================================
	
	/** An parallel octree with shallow morton code and usual data structures for front and front insertion container.  */
	template< typename Point >
	using ShallowParallelOctree = ParallelOctree< unsigned int, Point, unordered_set< MortonCode< unsigned int > >,
												  unordered_set< MortonCode< unsigned int > > >;
}

#endif