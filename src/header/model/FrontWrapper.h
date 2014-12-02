#ifndef FRONT_WRAPPER_H
#define FRONT_WRAPPER_H

namespace model
{
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point, typename Front >
	class FrontOctree;
	
	/** Wrapper used to specialize just some parts of the front behavior. */
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point, typename Front >
	struct FrontWrapper {};
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	struct FrontWrapper< MortonPrecision, Float, Vec3, Point, unordered_set< MortonCode< MortonPrecision > > >
	{
		using MortonCode = model::MortonCode< MortonPrecision >;
		using MortonCodePtr = model::MortonCodePtr< MortonPrecision >;
		using MortonVector = vector< MortonCode >;
		using MortonPtrVector = vector< MortonCodePtr >;
		using RenderingState = model::RenderingState< Vec3 >;
		using Front = unordered_set< MortonCode >;
		using FrontOctree = model::FrontOctree< MortonPrecision, Float, Vec3, Point, Front >;
		
		static void trackFront( FrontOctree& octree, RenderingState& renderingState, const Float& projThresh )
		{
			// Flag to indicate if the previous front entry should be deleted. This can be done only after the
			// iterator to the entry to be deleted is incremented. Otherwise the iterator will be invalidated before
			// increment.
			bool erasePrevious = false;
			
			Front& front = octree.m_front;
			
			typename Front::iterator end = front.end();
			typename Front::iterator prev;
			
			for( typename Front::iterator it = front.begin(); it != end; prev = it, ++it,
				end = front.end() )
			{
				//cout << endl << "Current: " << hex << it->getBits() << dec << endl;
				if( erasePrevious )
				{
					//cout << "Erased: " << hex << prev->getBits() << dec << endl;
					front.erase( prev );
				}
				//erasePrevious = false;
				
				MortonCodePtr code = make_shared< MortonCode >( *it );
				
				erasePrevious = octree.trackNode( code, renderingState, projThresh );
			}
			
			// Delete the node from last iteration, if necessary.
			if( erasePrevious )
			{
				//cout << "Erased: " << hex << prev->getBits() << dec << endl;
				octree.m_front.erase( prev );
			}
		}
		
		static void prune( FrontOctree& octree, const MortonCodePtr& code )
		{
			MortonPtrVector deletedCodes = code->traverseUp()->traverseDown();
		
			Front& front = octree.m_front;
			
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