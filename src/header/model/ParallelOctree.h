#ifndef PARALLEL_OCTREE_H
#define PARALLEL_OCTREE_H

#include <omp.h>
#include "FrontOctree.h"
#include "ParallelFrontBehavior.h"

namespace model
{
	/** An octree that performs front tracking in parallel. */
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point, typename FrontBehavior >
	class ParallelOctree
	: public FrontOctree< MortonPrecision, Float, Vec3, Point, FrontBehavior >
	{
		using MortonCode = model::MortonCode< MortonPrecision >;
		using MortonVector = vector< MortonCode >;
		using PointPtr = shared_ptr< Point >;
		using PointVector = vector< PointPtr >;
		using PointVectorPtr = shared_ptr< PointVector >;
		using OctreeNodePtr = model::OctreeNodePtr< MortonPrecision, Float, Vec3 >;
		using FrontOctree = model::FrontOctree< MortonPrecision, Float, Vec3, Point, FrontBehavior >;
		using ParallelFrontBehavior = model::ParallelFrontBehavior< MortonPrecision, Float, Vec3, Point >;
		using RandomSampleOctree = model::RandomSampleOctree< MortonPrecision, Float, Vec3, Point >;
		using RenderingState = model::RenderingState< Vec3, Float >;
	
	public:
		ParallelOctree( const int& maxPointsPerNode, const int& maxLevel );
		
	protected:
		virtual void setupNodeRendering( OctreeNodePtr node, RenderingState& renderingState );
		
	private:
		/** List with the nodes that will be deleted in current front tracking. */
		MortonVector m_frontDeletionList;
	};
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point, typename FrontBehavior >
	ParallelOctree< MortonPrecision, Float, Vec3, Point, FrontBehavior >::ParallelOctree(
		const int& maxPointsPerNode, const int& maxLevel )
	: FrontOctree( maxPointsPerNode, maxLevel )
	{
		FrontOctree::m_frontBehavior = new ParallelFrontBehavior( *this );
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point, typename FrontBehavior >
	inline void ParallelOctree< MortonPrecision, Float, Vec3, Point, FrontBehavior >::setupNodeRendering(
		OctreeNodePtr node, RenderingState& renderingState )
	{
		PointVectorPtr points = node-> template getContents< PointVector >();
		
		#pragma omp critical (FrontRendering)
			renderingState.handleNodeRendering( points );
	}
	
	//=====================================================================
	// Type Sugar.
	//=====================================================================
	
	template< typename Float, typename Vec3, typename Point, typename FrontBehavior >
	using ShallowParallelOctree = ParallelOctree< unsigned int, Float, Vec3, Point, FrontBehavior >;
	
	template< typename Float, typename Vec3, typename Point, typename FrontBehavior >
	using MediumParallelOctree = ParallelOctree< unsigned long, Float, Vec3, Point, FrontBehavior >;
}

#endif