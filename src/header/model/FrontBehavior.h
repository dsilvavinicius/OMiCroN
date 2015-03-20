#ifndef FRONT_BEHAVIOR_H
#define FRONT_BEHAVIOR_H

//#include "CompactionRenderingState.h"
#include "RenderingState.h"
#include "Front.h"

namespace model
{
	/** Just an unifier for some std container API. */
	struct ContainerAPIUnifier
	{
		template< typename Value >
		static void insert( vector< Value >& container, const Value& value )
		{
			container.push_back( value );
		}
		
		template< typename Value >
		static void insert( unordered_set< Value >& container, const Value& value )
		{
			container.insert( value );
		}
	};
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point, typename Front >
	class FrontOctree;
	
	/** Wrapper used to "specialize" just the parts of the front behavior in FrontOctree and derived classes. This struct
	 * should be tightly coupled with the class that it is "specializing". */
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point, typename Front,
			  typename InsertionContainer >
	class FrontBehavior
	{};
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point, typename InsertionContainer >
	class FrontBehavior< MortonPrecision, Float, Vec3, Point, unordered_set< MortonCode< MortonPrecision > >,
						 InsertionContainer >
	{
	public:
		using MortonCode = model::MortonCode< MortonPrecision >;
		using MortonCodePtr = model::MortonCodePtr< MortonPrecision >;
		using MortonVector = vector< MortonCode >;
		using MortonPtrVector = vector< MortonCodePtr >;
		using RenderingState = model::RenderingState< Vec3, Float >;
		using Front = unordered_set< MortonCode >;
		using FrontOctree = model::FrontOctree< MortonPrecision, Float, Vec3, Point, FrontBehavior >;
		
		FrontBehavior( FrontOctree& octree )
		: m_octree( octree ) {}
		
		/** Main front tracking method. */
		virtual void trackFront( RenderingState& renderingState, const Float& projThresh )
		{
			// Flag to indicate if the previous front entry should be deleted. This can be done only after the
			// iterator to the entry to be deleted is incremented. Otherwise the iterator will be invalidated before
			// increment.
			bool erasePrevious = false;
			
			typename Front::iterator end = m_front.end();
			typename Front::iterator prev;
			
			for( typename Front::iterator it = m_front.begin(); it != end; prev = it, ++it,
				end = m_front.end() )
			{
				//cout << endl << "Current: " << hex << it->getBits() << dec << endl;
				if( erasePrevious )
				{
					//cout << "Erased: " << hex << prev->getBits() << dec << endl;
					m_front.erase( prev );
				}
				//erasePrevious = false;
				
				MortonCodePtr code = make_shared< MortonCode >( *it );
				
				erasePrevious = m_octree.trackNode( code, renderingState, projThresh );
			}
			
			// Delete the node from last iteration, if necessary.
			if( erasePrevious )
			{
				//cout << "Erased: " << hex << prev->getBits() << dec << endl;
				m_front.erase( prev );
			}
		}
		
		/** Prune all siblings of a node ( the node itself is not affected ). */
		virtual void prune( const MortonCodePtr& code )
		{
			MortonPtrVector deletedCodes = code->traverseUp()->traverseDown();
			
			for( MortonCodePtr deletedCode : deletedCodes )
			{
				if( *deletedCode != *code )
				{
					//cout << "Prunning: " << hex << deletedCode->getBits() << dec << endl;
					
					typename Front::iterator it = m_front.find( *deletedCode );
					if( it != m_front.end()  )
					{
						//cout << "Pruned: " << hex << deletedCode->getBits() << dec << endl;
						m_front.erase( it );
					}
				}
			}
		}
		
		/** Mark a node for insertion in front. */
		virtual void insert( const MortonCode& code )
		{
			ContainerAPIUnifier::insert( m_insertionList, code );
		}
		
		/** Inserts all nodes marked for insertion into front. */
		virtual void onFrontTrackingEnd()
		{
			m_front.insert( m_insertionList.begin(), m_insertionList.end() );
		}
		
		/** Clears the data structures related with the marked nodes. */
		virtual void clearMarkedNodes()
		{
			m_insertionList.clear();
		}
		
		int size() { return m_front.size(); }
		
	protected:
		FrontOctree& m_octree;
		
		/** Hierarchy front. */
		Front m_front;
		
		/** List with the nodes that will be included in current front tracking. */
		InsertionContainer m_insertionList;
	};
	
	template< typename Float, typename Vec3, typename Point, typename Front, typename InsertionContainer >
	using ShallowFrontBehavior = FrontBehavior< unsigned int, Float, Vec3, Point, Front, InsertionContainer >;
}

#endif