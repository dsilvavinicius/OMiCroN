#ifndef FRONT_OCTREE_H
#define FRONT_OCTREE_H

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>

#include "RandomSampleOctree.h"

//using boost::multi_index_container;
//using namespace ::boost;
//using namespace boost::multi_index;

namespace model
{
	/** Bidirectional map with an ordered index and a hashed index. */
	/*template< typename FromType, typename ToType >
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
	};*/
	
	/** Hierarchy front. The front is formed by all nodes in which the hierarchy traversal ends. */
	/*template< typename MortonPrecision >
	using Front = typename BidirectionalMap< unsigned long, MortonCodePtr< MortonPrecision > >::Container;
	
	using ShallowFront = Front< unsigned int >;
	using DeepFront = Front< unsigned long >;*/

	/** Octree that supports temporal coherence by hierarchy front tracking.  */
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	class FrontOctree
	: public RandomSampleOctree< MortonPrecision, Float, Vec3, Point >
	{
		using MortonCodePtr = model::MortonCodePtr< MortonPrecision >;
		using Front = set< MortonCodePtr >;
		using RandomSampleOctree = model::RandomSampleOctree< MortonPrecision, Float, Vec3, Point >;
		using RenderingState = model::RenderingState< Vec3 >;
		using MortonVector = vector< MortonCodePtr >;
		using OctreeNodePtr = model::OctreeNodePtr< MortonPrecision, Float, Vec3 >;
		using PointVector = model::PointVector< Float, Vec3 >;
		using PointVectorPtr = model::PointVectorPtr< Float, Vec3 >; 
	public:
		FrontOctree( const int& maxPointsPerNode, const int& maxLevel );
		
		/** Tracks the hierarchy front. */
		unsigned int trackFront( QGLPainter* painter, const Attributes& attribs, const Float& projThresh );
		
		/** Checks if the node and their siblings should be pruned from the front, giving place to their parent. */
		bool checkPrune( RenderingState& renderingState, const MortonCodePtr& code, const Float& projThresh ) const;
		
		/** Creates the deletion and insertion entries related with the prunning of the node and their siblings. */
		void prune( const MortonCodePtr& code, RenderingState& renderingState );
		
		/** Check if the node should be branched, giving place to its children. */
		bool checkBranch( RenderingState& renderingState, const MortonCodePtr& code, const Float& projThresh ) const;
		
		/** Creates the deletion and insertion entries related with the branching of the node. */
		void branch( const MortonCodePtr& code );
		
		void setupLeafNodeRendering( OctreeNodePtr leafNode, MortonCodePtr code, RenderingState& renderingState );
		
		void setupInnerNodeRendering( OctreeNodePtr innerNode, MortonCodePtr code, RenderingState& renderingState );
	
	private:
		/** Internal setup method for both leaf and inner node cases. */
		void setupNodeRendering( OctreeNodePtr node, MortonCodePtr code, RenderingState& renderingState );
		
		/** Hierarchy front. */
		Front m_front;
		
		/** List with the nodes that will be deleted in current front tracking. */
		//MortonVector frontDeletionList;
		
		/** List with the nodes that will be included in current front tracking. */
		//MortonVector frontInsertionList;
		
		/** The projection threshold used in the last frame. Used to track the front. */
		//Float m_lastProjThresh;
	};
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	FrontOctree< MortonPrecision, Float, Vec3, Point >::FrontOctree( const int& maxPointsPerNode, const int& maxLevel )
	: RandomSampleOctree::RandomSampleOctree( maxPointsPerNode, maxLevel )
	{}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	unsigned int FrontOctree< MortonPrecision, Float, Vec3, Point >::trackFront( QGLPainter* painter, const Attributes& attribs,
																			  const Float& projThresh )
	{
		RenderingState renderingState( painter, attribs );
		
		//frontDeletionList.clear();
		//frontInsertionList.clear();
		
		for( typename Front::iterator it = m_front.begin(); it != m_front.end(); ++it )
		{
			MortonCodePtr code = *it;
			
			bool oneLevelPruneDone = checkPrune( renderingState, code, projThresh );
			if( oneLevelPruneDone )
			{
				prune( code, renderingState );
				code = code->traverseUp();
				while( checkPrune( renderingState, code, projThresh ) )
				{
					prune( code, renderingState );
					code = code->traverseUp();
				}
			}
			else if( checkBranch( renderingState, code, projThresh ) )
			{
				RandomSampleOctree::traverse( code, renderingState, projThresh );
			}
			else
			{
				// No prunning or branching done. Just send the current front node for rendering.
				auto nodeIt = RandomSampleOctree::m_hierarchy->find( code );
				assert( nodeIt != m_hierarchy->end() );
				
				OctreeNodePtr node = nodeIt->second;
				PointVectorPtr points = node-> template getContents< PointVector >();
				renderingState.handleNodeRendering( renderingState, points );
			}
		}
		
		return renderingState.render();
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	inline void FrontOctree< MortonPrecision, Float, Vec3, Point >::prune( const MortonCodePtr& code,
																		   RenderingState& renderingState )
	{
		MortonCodePtr insertedCode = code->traverseUp();
		MortonVector deletedCodes = insertedCode->traverseDown();
		
		for( MortonCodePtr deletedNode : deletedCodes )
		{
			m_front.erase( deletedNode );
		}
		
		auto nodeIt = RandomSampleOctree::m_hierarchy->find( insertedCode );
		assert( nodeIt != m_hierarchy->end() );
		OctreeNodePtr insertedNode = nodeIt->second;
		setupNodeRendering( insertedNode, insertedCode, renderingState );
		
		//frontDeletionList.insert( frontDeletionList.end(), deletedNodes.begin(), deletedNodes.end() );
		//frontInsertionList.push_back( insertedNode );
	}
	
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	inline bool FrontOctree< MortonPrecision, Float, Vec3, Point >::checkPrune( RenderingState& renderingState,
																				const MortonCodePtr& code,
																			 const Float& projThresh ) const
	{
		MortonCodePtr parent = code->traverseUp();
		QBox3D box = RandomSampleOctree::getBoundaries( parent );
		bool parentIsCullable = RandomSampleOctree::isCullable( box, renderingState );
		
		if( parentIsCullable )
		{
			return true;
		}
		
		bool parentIsRenderable = RandomSampleOctree::isRenderable( box, renderingState, projThresh );
		if( !parentIsRenderable )
		{
			return false;
		}
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	inline bool FrontOctree< MortonPrecision, Float, Vec3, Point >::checkBranch( RenderingState& renderingState,
																				 const MortonCodePtr& code,
																			  const Float& projThresh ) const
	{
		QBox3D box = RandomSampleOctree::getBoundaries( code );
		return  !RandomSampleOctree::isRenderable( box, renderingState, projThresh ) &&
				!RandomSampleOctree::isCullable( box, renderingState );
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	inline void FrontOctree< MortonPrecision, Float, Vec3, Point >::setupLeafNodeRendering( OctreeNodePtr leafNode,
																							MortonCodePtr code,
																						 RenderingState& renderingState )
	{
		assert( leafNode->isLeaf() && "leafNode cannot be inner." );
		
		setupNodeRendering( leafNode, code, renderingState );
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	inline void FrontOctree< MortonPrecision, Float, Vec3, Point >::setupInnerNodeRendering( OctreeNodePtr innerNode,
																							 MortonCodePtr code,
																						  RenderingState& renderingState )
	{
		assert( !innerNode->isLeaf() && "innerNode cannot be leaf." );
		
		setupNodeRendering( innerNode, code, renderingState );
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	inline void FrontOctree< MortonPrecision, Float, Vec3, Point >::setupNodeRendering( OctreeNodePtr node, MortonCodePtr code,
																						RenderingState& renderingState )
	{
		m_front.insert( code );
		
		PointVectorPtr points = node-> template getContents< PointVector >();
		renderingState.handleNodeRendering( renderingState, points );
	}
	
	//=====================================================================
	// Type Sugar.
	//=====================================================================
	
	template< typename Float, typename Vec3, typename Point >
	using ShallowFrontOctree = FrontOctree< unsigned int, Float, Vec3, Point >;
	
	template< typename Float, typename Vec3, typename Point >
	using ShallowFrontOctreePtr = shared_ptr< ShallowFrontOctree< Float, Vec3, Point > >;
	
	template< typename Float, typename Vec3, typename Point >
	using MediumFrontOctree = FrontOctree< unsigned long, Float, Vec3, Point >;
	
	template< typename Float, typename Vec3, typename Point >
	using MediumFrontOctreePtr = shared_ptr< MediumFrontOctree< Float, Vec3, Point > >;
}

#endif