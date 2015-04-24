#ifndef FRONT_OCTREE_H
#define FRONT_OCTREE_H

#include <unordered_set>

#include "FrontBehavior.h"
//#include "RandomSampleOctree.h"
#include "IndexedOctree.h"

namespace model
{
	/** Octree that supports temporal coherence by hierarchy front tracking. Nodes that are added to a insertion container
	 *	while traversing and added to the actual front at traversal ending.
	 *	@param: Front is the type used as the actual front container.
	 *	@param: FrontInsertionContainer is the insertion container, used to track nodes that should be added to the front. */
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point, typename Front,
			  typename FrontInsertionContainer >
	class FrontOctree
	: public RandomSampleOctree< MortonPrecision, Float, Vec3, Point >
	{
		using MortonCode = model::MortonCode< MortonPrecision >;
		using MortonCodePtr = model::MortonCodePtr< MortonPrecision >;
		using ParentOctree = model::RandomSampleOctree< MortonPrecision, Float, Vec3, Point >;
		using RenderingState = model::RenderingState< Vec3, Float >;
		using TransientRenderingState = model::TransientRenderingState< Vec3, Float >;
		using MortonPtrVector = vector< MortonCodePtr >;
		using MortonVector = vector< MortonCode >;
		using OctreeNodePtr = model::OctreeNodePtr< MortonPrecision, Float, Vec3 >;
		using PointPtr = shared_ptr< Point >;
		using PointVector = vector< PointPtr >;
		using PointVectorPtr = shared_ptr< PointVector >;
		using FrontBehavior = model::FrontBehavior< MortonPrecision, Float, Vec3, Point, Front, FrontInsertionContainer >;
		
	public:
		FrontOctree( const int& maxPointsPerNode, const int& maxLevel );
		
		~FrontOctree();
		
		/** Tracks the hierarchy front, by prunning or branching nodes ( one level only ). This method should be called
		 * after ParentOctree::traverse( RenderingState& renderer, const Float& projThresh ),
		 * so the front can be init in a traversal from root. */
		FrontOctreeStats trackFront( RenderingState& renderer, const Float& projThresh );
	
		/** Tracks one node of the front.
		 * @returns true if the node represented by code should be deleted or false otherwise. */
		bool trackNode( MortonCodePtr& code, RenderingState& renderingState, const Float& projThresh );
		
	protected:
		/** Checks if the node and their siblings should be pruned from the front, giving place to their parent. */
		bool checkPrune( RenderingState& renderingState, const MortonCodePtr& code, const Float& projThresh ) const;
		
		/** Creates the deletion and insertion entries related with the prunning of the node and their siblings. */
		void prune( const MortonCodePtr& code );
		
		/** Check if the node should be branched, giving place to its children.
		 * @param isCullable is an output that indicates if the node was culled by frustrum. */
		bool checkBranch( RenderingState& renderingState, const MortonCodePtr& code, const Float& projThresh,
			bool& out_isCullable ) const;
		
		/** Overriden to add rendered node into front addition list. */
		void setupLeafNodeRendering( OctreeNodePtr leafNode, MortonCodePtr code, RenderingState& renderingState );
		
		/** Overriden to add rendered node into front addition list. */
		void setupInnerNodeRendering( OctreeNodePtr innerNode, MortonCodePtr code, RenderingState& renderingState );
		
		/** Overriden to add culled node into front addition list. */
		void handleCulledNode( MortonCodePtr code );
		
		/** Overriden to push the front addition list to the front itself. */
		void onTraversalEnd();
		
		/** Rendering setup method for both leaf and inner node cases. Adds the node into the front. */
		void setupNodeRendering( OctreeNodePtr node, MortonCodePtr code, RenderingState& renderingState );
		
		/** Rendering setup method that leaves the front insertion responsibility to the caller. Can be also used in cases
		 *	that insertion the node into front is not necessary. */
		//virtual void setupNodeRendering( OctreeNodePtr node, RenderingState& renderingState );
		
		/** Object with data related behavior of the front. */
		FrontBehavior* m_frontBehavior;
	};
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point, typename Front,
			  typename FrontInsertionContainer >
	FrontOctree< MortonPrecision, Float, Vec3, Point, Front, FrontInsertionContainer >::FrontOctree(
		const int& maxPointsPerNode, const int& maxLevel )
	: ParentOctree::RandomSampleOctree( maxPointsPerNode, maxLevel )
	{
		m_frontBehavior = new FrontBehavior( *this );
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point, typename Front,
			  typename FrontInsertionContainer >
	FrontOctree< MortonPrecision, Float, Vec3, Point, Front, FrontInsertionContainer >::~FrontOctree()
	{
		delete m_frontBehavior;
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point, typename Front,
			  typename FrontInsertionContainer >
	FrontOctreeStats FrontOctree< MortonPrecision, Float, Vec3, Point, Front, FrontInsertionContainer >::trackFront(
		RenderingState& renderer, const Float& projThresh )
	{
		clock_t timing = clock();
		
		//cout << "========== Starting front tracking ==========" << endl;
		m_frontBehavior->clearMarkedNodes();
		
		//
		/*cout << "Front: " << endl;
		for( MortonCode code : m_front )
		{
			cout << hex << code.getBits() << dec << endl;
		}*/
		//
		
		m_frontBehavior->trackFront( renderer, projThresh );
		
		onTraversalEnd();
		
		timing = clock() - timing;
		float traversalTime = float( timing ) / CLOCKS_PER_SEC * 1000;
		
		timing = clock();
		
		unsigned int numRenderedPoints = renderer.render();
		
		timing = clock() - timing;
		float renderingTime = float( timing ) / CLOCKS_PER_SEC * 1000;
		
		//cout << "========== Ending front tracking ==========" << endl << endl;
		
		return FrontOctreeStats( traversalTime, renderingTime, numRenderedPoints, m_frontBehavior->size() );
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point, typename Front,
			  typename FrontInsertionContainer >
	inline bool FrontOctree< MortonPrecision, Float, Vec3, Point, Front, FrontInsertionContainer >::trackNode(
		MortonCodePtr& code, RenderingState& renderingState, const Float& projThresh )
	{
		bool isCullable = false;
		bool eraseNode = false;
		
		// Code for prunnable front
		if( checkPrune( renderingState, code, projThresh ) )
		{
			//cout << "Prune" << endl;
			eraseNode = true;
			prune( code );
			code = code->traverseUp();
			
			auto nodeIt = ParentOctree::m_hierarchy->find( code );
			assert( nodeIt != ParentOctree::m_hierarchy->end() );
			OctreeNodePtr node = nodeIt->second;
			setupNodeRendering( node, code, renderingState );
		}
		else if( checkBranch( renderingState, code, projThresh, isCullable ) )
		{
			//cout << "Branch" << endl;
			eraseNode = false;
			
			MortonPtrVector children = code->traverseDown();
			
			for( MortonCodePtr child : children  )
			{
				auto nodeIt = ParentOctree::m_hierarchy->find( child );
				
				if( nodeIt != ParentOctree::m_hierarchy->end() )
				{
					eraseNode = true;
					//cout << "Inserted in front: " << hex << child->getBits() << dec << endl;
					m_frontBehavior->insert( *child );
					
					pair< Vec3, Vec3 > box = ParentOctree::getBoundaries( child );
					if( !renderingState.isCullable( box ) )
					{
						//cout << "Point set to render: " << hex << child->getBits() << dec << endl;
						OctreeNodePtr node = nodeIt->second;
						ParentOctree::setupNodeRendering( node, renderingState );
					}
				}
			}
			
			if( !eraseNode )
			{
				auto nodeIt = ParentOctree::m_hierarchy->find( code );
				assert( nodeIt != ParentOctree::m_hierarchy->end() );
				
				OctreeNodePtr node = nodeIt->second;
				ParentOctree::setupNodeRendering( node, renderingState );
			}
		}
		else
		{
			//cout << "Still" << endl;
			if( !isCullable )
			{
				// No prunning or branching done. Just send the current front node for rendering.
				auto nodeIt = ParentOctree::m_hierarchy->find( code );
				assert( nodeIt != ParentOctree::m_hierarchy->end() );
				
				OctreeNodePtr node = nodeIt->second;
				ParentOctree::setupNodeRendering( node, renderingState );
			}
		}
		
		return eraseNode;
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point, typename Front,
			  typename FrontInsertionContainer >
	inline void FrontOctree< MortonPrecision, Float, Vec3, Point, Front, FrontInsertionContainer >::prune(
		const MortonCodePtr& code )
	{
		m_frontBehavior->prune( code );
	}
	
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point, typename Front,
			  typename FrontInsertionContainer >
	inline bool FrontOctree< MortonPrecision, Float, Vec3, Point, Front, FrontInsertionContainer >::checkPrune(
		RenderingState& renderingState, const MortonCodePtr& code, const Float& projThresh ) const
	{
		if( code->getBits() == 1 )
		{	// Don't prune the root node.
			return false;
		}
		
		MortonCodePtr parent = code->traverseUp();
		pair< Vec3, Vec3 > box = ParentOctree::getBoundaries( parent );
		bool parentIsCullable = renderingState.isCullable( box );
		
		if( parentIsCullable )
		{
			return true;
		}
		
		bool parentIsRenderable = renderingState.isRenderable( box, projThresh );
		if( !parentIsRenderable )
		{
			return false;
		}
		
		return true;
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point, typename Front,
			  typename FrontInsertionContainer >
	inline bool FrontOctree< MortonPrecision, Float, Vec3, Point, Front, FrontInsertionContainer >::checkBranch(
		RenderingState& renderingState, const MortonCodePtr& code, const Float& projThresh, bool& out_isCullable )
		const
	{
		pair< Vec3, Vec3 > box = ParentOctree::getBoundaries( code );
		out_isCullable = renderingState.isCullable( box );
		
		return !renderingState.isRenderable( box, projThresh ) && !out_isCullable;
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point, typename Front,
			  typename FrontInsertionContainer >
	inline void FrontOctree< MortonPrecision, Float, Vec3, Point, Front, FrontInsertionContainer >::setupLeafNodeRendering(
		OctreeNodePtr leafNode, MortonCodePtr code, RenderingState& renderingState )
	{
		assert( leafNode->isLeaf() && "leafNode cannot be inner." );
		
		setupNodeRendering( leafNode, code, renderingState );
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point, typename Front,
			  typename FrontInsertionContainer >
	inline void FrontOctree< MortonPrecision, Float, Vec3, Point, Front, FrontInsertionContainer >::setupInnerNodeRendering(
		OctreeNodePtr innerNode, MortonCodePtr code, RenderingState& renderingState )
	{
		assert( !innerNode->isLeaf() && "innerNode cannot be leaf." );
		
		setupNodeRendering( innerNode, code, renderingState );
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point, typename Front,
			  typename FrontInsertionContainer >
	inline void FrontOctree< MortonPrecision, Float, Vec3, Point, Front, FrontInsertionContainer >::setupNodeRendering(
		OctreeNodePtr node, MortonCodePtr code, RenderingState& renderingState )
	{
		//cout << "Inserted draw: " << hex << code->getBits() << dec << endl;
		m_frontBehavior->insert( *code );
		
		ParentOctree::setupNodeRendering( node, renderingState );
	}
	
	/*template< typename MortonPrecision, typename Float, typename Vec3, typename Point, typename Front,
			  typename FrontInsertionContainer >
	inline void FrontOctree< MortonPrecision, Float, Vec3, Point, Front, FrontInsertionContainer >::setupNodeRendering(
		OctreeNodePtr node, RenderingState& renderingState )
	{
		PointVectorPtr points = node-> template getContents< PointVector >();
		renderingState.handleNodeRendering( points );
	}*/
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point, typename Front,
			  typename FrontInsertionContainer >
	inline void FrontOctree< MortonPrecision, Float, Vec3, Point, Front, FrontInsertionContainer >::handleCulledNode(
		MortonCodePtr code )
	{
		//cout << "Inserted cull: " << hex << code->getBits() << dec << endl;
		m_frontBehavior->insert( *code );
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point, typename Front,
			  typename FrontInsertionContainer >
	void FrontOctree< MortonPrecision, Float, Vec3, Point, Front, FrontInsertionContainer >::onTraversalEnd()
	{
		m_frontBehavior->onFrontTrackingEnd();
	}
	
	//=====================================================================
	// Type Sugar.
	//=====================================================================
	
	/** An front octree with shallow morton code and usual data structures for front and front insertion container. */
	template< typename Float, typename Vec3, typename Point >
	using ShallowFrontOctree = FrontOctree< unsigned int, Float, Vec3, Point, unordered_set< MortonCode< unsigned int > >,
											vector< MortonCode< unsigned int > > >;
}

#endif