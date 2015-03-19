#ifndef PARALLEL_FRONT_BEHAVIOR_H

#include <omp.h>
#include "FrontBehavior.h"

namespace model
{
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point, typename Front >
	class ParallelFrontOctree;
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	class ParallelFrontBehavior
	: FrontBehavior< MortonPrecision, Float, Vec3, Point, unordered_set< MortonCode< MortonPrecision > > >
	{
		using MortonCode = model::MortonCode< MortonPrecision >;
		using MortonCodePtr = model::MortonCodePtr< MortonPrecision >;
		using MortonVector = vector< MortonCode >;
		using MortonPtrVector = vector< MortonCodePtr >;
		using RenderingState = model::RenderingState< Vec3, Float >;
		using Front = unordered_set< MortonCode >;
		using FrontOctree = model::ParallelFrontOctree< MortonPrecision, Float, Vec3, Point, Front >;
		using FrontBehavior = model::FrontBehavior< MortonPrecision, Float, Vec3, Point,
													unordered_set< MortonCode< MortonPrecision > > >;
	
	public:
		void trackFront( RenderingState& renderingState, const Float& projThresh )
		{
			Front& front = FrontBehavior::m_octree.m_front;
			
			#pragma omp parallel num_threads( 8 )
			{
				int id = omp_get_thread_num();
				int nThreads = omp_get_num_threads();
				
				#pragma omp critical FrontDeletionReading
					typename Front::iterator it = advance( front.begin(), id );
				
				while( it != front.end() )
				{
					MortonCodePtr code = make_shared< MortonCode >( *it );
					
					FrontBehavior::m_octree.trackNode( code, renderingState, projThresh );
					
					#pragma omp critical FrontDeletionReading
						advance( it, nThreads);
				}
			}
		}
		
		void prune( const MortonCodePtr& code )
		{
			MortonPtrVector deletedCodes = code->traverseUp()->traverseDown();
		
			Front& front = FrontBehavior::m_octree.m_front;
			
			for( MortonCodePtr deletedCode : deletedCodes )
			{
				if( *deletedCode != *code )
				{
					//cout << "Prunning: " << hex << deletedCode->getBits() << dec << endl;
					
					#pragma omp critical FrontDeletionReading
					{
						typename Front::iterator it = front.find( *deletedCode );
						if( it != front.end()  )
						{
							//cout << "Pruned: " << hex << deletedCode->getBits() << dec << endl;
							front.erase( it );
						}
					}
				}
			}
		}
		
		void insert( const MortonVector& toBeInserted )
		{
			Front front = FrontBehavior::m_octree.m_front;
			
			front.insert( toBeInserted.begin(), toBeInserted.end() );
		}
	};
}

#endif