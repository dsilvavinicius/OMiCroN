#ifndef PARALLEL_FRONT_BEHAVIOR_H
#define PARALLEL_FRONT_BEHAVIOR_H

#include <omp.h>
#include <unordered_set>
#include "FrontBehavior.h"

namespace model
{
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point, typename Front,
			  typename FrontInsertionContainer >
	class FrontOctree;
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point, typename Front,
			  typename FrontInsertionContainer >
	class ParallelFrontBehavior{};
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	class ParallelFrontBehavior< MortonPrecision, Float, Vec3, Point, unordered_set< MortonCode< MortonPrecision > >,
								 unordered_set< MortonCode< MortonPrecision > > >
	: public FrontBehavior< MortonPrecision, Float, Vec3, Point, unordered_set< MortonCode< MortonPrecision > >,
							unordered_set< MortonCode< MortonPrecision > > >
	{
		using MortonCode = model::MortonCode< MortonPrecision >;
		using MortonCodePtr = model::MortonCodePtr< MortonPrecision >;
		using MortonVector = vector< MortonCode >;
		using MortonPtrVector = vector< MortonCodePtr >;
		using RenderingState = model::RenderingState< Vec3, Float >;
		using Front = unordered_set< MortonCode >;
		using InsertionContainer = unordered_set< MortonCode >;
		using FrontBehavior = model::FrontBehavior< MortonPrecision, Float, Vec3, Point, Front, InsertionContainer >;
		using FrontOctree = model::FrontOctree< MortonPrecision, Float, Vec3, Point, Front, InsertionContainer >;
	
	public:
		ParallelFrontBehavior( FrontOctree& octree )
		: FrontBehavior( octree ) {}
		
		void trackFront( RenderingState& renderingState, const Float& projThresh )
		{
			#pragma omp parallel num_threads( 8 )
			{
				int id = omp_get_thread_num();
				int nThreads = omp_get_num_threads();
				
				typename Front::iterator it = FrontBehavior::m_front.begin();
				advance( it, id );
				while( it != FrontBehavior::m_front.end() )
				{
					MortonCodePtr code = make_shared< MortonCode >( *it );
					
					FrontBehavior::m_octree.trackNode( code, renderingState, projThresh );
					
					advance( it, nThreads);
				}
			}
		}
		
		void prune( const MortonCodePtr& code )
		{
			#pragma omp critical (FrontDeletion)
				m_deletionList.push_back( *code );
		}
		
		/** Mark a node for insertion in front. */
		virtual void insert( const MortonCode& code )
		{
			#pragma omp critical (FrontInsertion)
				ContainerAPIUnifier::insert( FrontBehavior::m_insertionList, code );
		}
		
		/** Inserts and deletes from front all nodes marked for insertion and deletion respectively. */
		virtual void onFrontTrackingEnd()
		{
			for( MortonCode& code : m_deletionList )
			{
				FrontBehavior::m_front.erase( code );
			}
			
			FrontBehavior::m_front.insert( FrontBehavior::m_insertionList.begin(),
										   FrontBehavior::m_insertionList.end() );
		}
		
		/** Clears the data structures related with the marked nodes. */
		virtual void clearMarkedNodes()
		{
			FrontBehavior::m_insertionList.clear();
			m_deletionList.clear();
		}
	
	private:
		/** List with front nodes to be deleted. */
		MortonVector m_deletionList;
	};
	
	//=====================================================================
	// Type Sugar.
	//=====================================================================
	
	/** Front behavior with shallow morton code and usual datastructures that allow parallelism. */
	template< typename Float, typename Vec3, typename Point >
	using ShallowParallelFrontBehavior = ParallelFrontBehavior< unsigned int, Float, Vec3, Point,
																unordered_set< MortonCode< unsigned int > >,
																unordered_set< MortonCode< unsigned int > >
															  >;
}

#endif