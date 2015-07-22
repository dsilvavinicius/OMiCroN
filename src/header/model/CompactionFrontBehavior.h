#ifndef COMPACTION_FRONT_BEHAVIOR_H
#define COMPACTION_FRONT_BEHAVIOR_H

#include "FrontBehavior.h"

namespace model
{
	template< typename MortonCode, typename Point >
	class CompactionFrontOctree;
	
	template< typename MortonCode, typename Point >
	struct FrontWrapper< MortonCode, Point, typename FrontTypes< MortonCode >::Front >
	{
		using MortonCodePtr = shared_ptr< MortonCode >;
		using MortonVector = vector< MortonCode >;
		using MortonPtrVector = vector< MortonCodePtr >;
		using RenderingState = model::CompactionRenderingState;
		using FrontTypes = model::FrontTypes< MortonCode >;
		using Front = typename FrontTypes::Front;
		using FrontOctree = model::CompactionFrontOctree< MortonCode, Point >;
		
		static void trackFront( FrontOctree& octree, RenderingState& renderingState, const Float& projThresh )
		{
			// Flag to indicate if the previous front entry should be deleted. This can be done only after the
			// iterator to the entry to be deleted is incremented. Otherwise the iterator will be invalidated before
			// increment.
			bool erasePrevious = false;
			
			typename FrontTypes::FrontBySequence front = octree.m_front.get< FrontTypes::sequential >(); 
			typename FrontTypes::FrontBySequence::iterator end = front.end();
			typename FrontTypes::FrontBySequence::iterator prev;
			
			for( typename FrontTypes::FrontBySequence::iterator it = front.begin(); it != end; prev = it, ++it,
				end = front.end() )
			{
				//cout << endl << "Current: " << hex << it->getBits() << dec << endl;
				if( erasePrevious )
				{
					//cout << "Erased: " << hex << prev->getBits() << dec << endl;
					front.erase( prev );
				}
				//erasePrevious = false;
				
				MortonCodePtr code( new MortonCode( *it ) );
				
				erasePrevious = octree.trackNode( code, renderingState, projThresh );
			}
			
			// Delete the node from last iteration, if necessary.
			if( erasePrevious )
			{
				//cout << "Erased: " << hex << prev->getBits() << dec << endl;
				front.erase( prev );
			}
		}
		
		static void prune( FrontOctree& octree, const MortonCodePtr& code )
		{
			MortonPtrVector deletedCodes = code->traverseUp()->traverseDown();
		
			typename FrontTypes::FrontBySequence front = octree.m_front.get< FrontTypes::morton >(); 
			
			for( MortonCodePtr deletedCode : deletedCodes )
			{
				if( *deletedCode != *code )
				{
					//cout << "Prunning: " << hex << deletedCode->getBits() << dec << endl;
					
					typename Front::iterator it = front.find( *deletedCode );
					if( it != front.end()  )
					{
						//cout << "Pruned: " << hex << deletedCode->getBits() << dec << endl;
						front.erase( it );
					}
				}
			}
		}
		
		static void insert( FrontOctree& octree, const MortonVector& toBeInserted )
		{
			octree.m_front.insert( toBeInserted.begin(), toBeInserted.end() );
		}
	};
}

#endif