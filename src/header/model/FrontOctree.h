#ifndef FRONT_OCTREE
#define FRONT_OCTREE

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>

#include "RandomSampleOctree.h"

using boost::multi_index_container;
using namespace ::boost;
using namespace boost::multi_index;

namespace model
{
	/** Bidirectional map with an ordered index and a hashed index. */
	template< typename FromType, typename ToType >
	struct BidirectionalMap
	{
		struct from{};
		struct to{};
		
		struct ValueType
		{
			ValueType( const FromType& first, const ToType& second ):
			m_first( first ),
			m_second( second )
			{}

			FromType m_first;
			ToType   m_second;
		};
		
		using Container = multi_index_container<
			ValueType,
			indexed_by<
				ordered_unique<
					tag< from >, member< ValueType, FromType, &ValueType::m_first > >,
				hashed_unique<
					tag< to >,   member< ValueType, ToType,   &ValueType::m_second > >
			>
		>;
	};
	
	/** Hierarchy front. The front is formed by all nodes in which the hierarchy traversal ends. */
	template< typename MortonPrecision >
	using Front = typename BidirectionalMap< unsigned long, MortonCodePtr< MortonPrecision > >::Container;
	
	using ShallowFront = Front< unsigned int >;
	using DeepFront = Front< unsigned long >;

	/** Octree that supports temporal coherence by hierarchy front tracking.  */
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	class FrontOctree
	: public RandomSampleOctree< MortonPrecision, Float, Vec3, Point >
	{
		using MortonCodePtr = model::MortonCodePtr< MortonPrecision >;
		using Front = model::Front< MortonPrecision >;
		using RenderingState = model::RenderingState< Vec3 >;
		using MortonVector = vector< MortonCodePtr >;
	public:
		FrontOctree( const int& maxPointsPerNode, const int& maxLevel );
		
		/** Tracks the hierarchy front. */
		void trackFront( QGLPainter* painter, const Attributes& attribs, const Float& projThresh );
		
		/** Checks if the node and their siblings should be pruned from the front, giving place to their parent. */
		bool checkPrune( const RenderingState& renderingState, const MortonCodePtr& code, const Float& projThresh ) const;
		
		/** Creates the deletion and insertion entries related with the prunning of the node and their siblings. */
		void prune( const MortonCodePtr& code );
		
		/** Check if the node should be branched, giving place to its children. */
		bool checkBranch( const RenderingState& renderingState, const MortonCodePtr& code, const Float& projThresh ) const;
		
		/** Creates the deletion and insertion entries related with the branching of the node. */
		void branch( const MortonCodePtr& code );
		
		void setupLeafNodeRendering( OctreeNodePtr< MortonPrecision, Float, Vec3 > leafNode,
									 RenderingState& renderingState );
		
		void setupInnerNodeRendering( OctreeNodePtr< MortonPrecision, Float, Vec3 > innerNode,
									  RenderingState& renderingState );
	private:
		/** Hierarchy front. */
		Front m_front;
		
		/** List with the nodes that will be deleted in current front tracking. */
		MortonVector frontDeletionList;
		
		/** List with the nodes that will be included in current front tracking. */
		MortonVector frontInsertionList;
		
		/** The projection threshold used in the last frame. Used to track the front. */
		Float m_lastProjThresh;
	};
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	FrontOctree< MortonPrecision, Float, Vec3, Point >::FrontOctree( const int& maxPointsPerNode, const int& maxLevel )
	:RandomSampleOctree< MortonPrecision, Float, Vec3, Point >::Octree( maxPointsPerNode, maxLevel )
	{}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	void FrontOctree< MortonPrecision, Float, Vec3, Point >::trackFront( QGLPainter* painter, const Attributes& attribs,
																		 const Float& projThresh )
	{
		RenderingState renderingState( painter, attribs );
		
		frontDeletionList.clear();
		frontInsertionList.clear();
		
		for( typename Front::iterator it = m_front.begin(); it != m_front.end(); ++it )
		{
			MortonCodePtr code = it->right;
			
			bool oneLevelPruneDone = checkPrune( renderingState, code, projThresh );
			if( oneLevelPruneDone )
			{
				prune( code );
				code = code->traverseUp();
				while( checkPrune( renderingState, code, projThresh ) )
				{
					prune( code );
					code = code->traverseUp();
				}
			}
			else
			{
				if( checkBranch( renderingState, code, projThresh ) )
				{
					traverse( code, renderingState );
				}
			}
		}
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	inline void FrontOctree< MortonPrecision, Float, Vec3, Point >::prune( const MortonCodePtr& code )
	{
		MortonCodePtr insertedNode = code->traverseUp();
		MortonVector deletedNodes = insertedNode->traverseDown();
		
		frontDeletionList.insert( frontDeletionList.end(), deletedNodes.begin(), deletedNodes.end() );
		frontInsertionList.push_back( insertedNode );
	}
	
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	inline bool FrontOctree< MortonPrecision, Float, Vec3, Point >::checkPrune( const RenderingState& renderingState,
																				const MortonCodePtr& code,
																			 const Float& projThresh ) const
	{
		MortonCodePtr parent = code->traverseUp();
		QBox3D box = getBoundaries( parent );
		bool parentIsCullable = isCullable( box, renderingState );
		
		if( parentIsCullable )
		{
			return true;
		}
		
		bool parentIsRenderable = isRenderable( box, renderingState, projThresh );
		if( !parentIsRenderable )
		{
			return false;
		}
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	inline bool FrontOctree< MortonPrecision, Float, Vec3, Point >::checkBranch( const RenderingState& renderingState,
																				 const MortonCodePtr& code,
																			  const Float& projThresh ) const
	{
		QBox3D box = getBoundaries( code );
		return !isRenderable( box, renderingState, projThresh );
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	void FrontOctree< MortonPrecision, Float, Vec3, Point >::setupLeafNodeRendering(
		OctreeNodePtr< MortonPrecision, Float, Vec3 > innerNode, RenderingState& renderingState )
	{
		
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	void FrontOctree< MortonPrecision, Float, Vec3, Point >::setupInnerNodeRendering(
		OctreeNodePtr< MortonPrecision, Float, Vec3 > innerNode, RenderingState& renderingState )
	{
		
	}
}

#endif