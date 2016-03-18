#ifndef FRONT_H
#define FRONT_H

#include <forward_list>
#include "O1OctreeNode.h"
#include "OctreeDimensions.h"
#include "OctreeStats.h"
#include "RenderingState.h"

using namespace std;

namespace model
{
	/** Current visualization front of the hierarchy. */
	template< typename Morton, typename Point >
	class Front
	{
	public:
		using PointPtr = shared_ptr< Point >;
		using Node = O1OctreeNode< PointPtr >;
		using NodeArray = Array< Node >;
		using OctreeDim = OctreeDimensions< Morton, Point >;
		using Sql = SQLiteManager< Point, Morton, Node >;
		using Renderer = RenderingState;
		using FrontList = forward_list< FrontNode& >;
		using FrontListIter = typename FrontNode::iterator;
		
		/** The node type that is used in front. */
		typedef struct FrontNode
		{
			Node& m_octreeNode;
			unsigned char m_lvl;
		} FrontNode;
		
		/** Ctor.
		 * @param sortedPlyFilename is the path to a .ply file with points sorted in morton code order. */
		Front( const string& sortedPlyFilename );
		
		/** Synchronized. Inserts a leaf sibling group into front so it can be tracked from now on.
		 * @param leafSiblings is the sibling group array
		 * @param lvl is the hierarchy level which leafSiblings belongs to. */
		void insertIntoFront( NodeArray& leafSiblings, unsigned char lvl );
		
		/** Indicates that node release is needed. The front tracking will release nodes until a call to turnReleaseOff() occurs. */
		void turnReleaseOn();
		
		/** Indicates that node release is not needed anymore. The front tracking will stop the node release. */
		void turnReleaseOff();
		
		/** Persists and release nodes in order to ease memory pressure. */
		void releaseSiblings( NodeArray& siblings, const OctreeDim& dim );
		
		/** Tracks the front based on the projection threshold.
		 * @param renderer is the responsible of rendering the points of the tracked front.
		 * @param projThresh is the projection threashold */
		FrontOctreeStats trackFront( RenderingState& renderer, const Float& projThresh );
	
		void trackNode( RenderingState& renderingState, const Float& projThresh );
		
		bool checkPrune( RenderingState& renderingState, const Float& projThresh );
		
		void prune( RenderingState& renderingState );
		
	private:
		/** Front internal representation. */
		FrontList m_front;
		
		/** Database connection used to persist nodes. */
		Sql m_db;
		
		/** Mutex to synchronize insertions into front from other threads. */
		mutex m_frontInsertionMtx;
		
		/** Dimensions of the octree nodes at deepest level. */
		OctreeDim m_deepestLvlDim;
		
		/** true if release is on, false otherwise. */
		bool m_releaseFlag;
	};
	
	template< typename Morton, typename Point >
	inline Front< Morton, Point >::Front()
	{
	}
	
	template< typename Morton, typename Point >
	inline void Front< Morton, Point >::trackFront( Renderer& renderer, const Float& projThresh )
	{
		if( m_releaseFlag )
		{
			m_db.beginTransaction();
		}
		
		for( FrontListIter frontIter = m_front.before_begin(); next( frontIter ) != m_front.end();/* */)
		{
			trackNode( frontIter, renderer, projThresh );
		}
		
		if( m_releaseFlag )
		{
			m_db.endTransaction();
		}
	}
	
	template< typename Morton, typename Point >
	inline void Front< Morton, Point >::trackNode( FrontListIter iter, Renderer& renderer, const Float& projThresh )
	{
		FrontNode& node = *next( iter );
		FrontNode& parentNode = node.parent();
		OctreeDim nodeLvlDim( m_deepestLvlDim, node.m_lvl );
		Morton morton = nodeLvlDim.calcMorton( node.m_octreeNode );
		
		if( parentNode != nullptr )
		{
			OctreeDim parentLvlDim( m_deepestLvlDim, node.m_lvl - 1 );
			Morton parentMorton = *morton.traverseUp();
		
			if( checkPrune( parentMorton, parentLvlDim, renderer, projThresh ) )
			{
				prune( iter, nodeLvlDim, parentNode, renderer );
				return;
			}
		}
		
		bool isCullable = false;
		
		//cout << "Tracking: " << code->getPathToRoot( true ) << endl;
		if( checkBranch( renderer, morton, projThresh, isCullable ) )
		{
			//cout << "Branching" << endl << endl;
			branch( morton, renderer );
			return;
		}
		
		if( !isCullable )
		{
			// No prunning or branching done. Just send the current front node for rendering.
			auto nodeIt = ParentOctree::m_hierarchy->find( morton );
			assert( nodeIt != ParentOctree::m_hierarchy->end() );
			
			OctreeNodePtr node = nodeIt->second;
			setupNodeRenderingNoFront( node, morton, renderer );
		}
	}
	
	template< typename Morton, typename Point >
	inline bool Front< Morton, Point >::checkPrune( const Morton& parentMorton, const OctreeDim& parentLvlDim,
													Renderer& renderer, const Float& projThresh ) const
	{
		pair< Vec3, Vec3 > box = parentLvlDim.getBoundaries( parentMorton );
		
		if( renderer.isCullable( box ) )
		{
			return true;
		}
		
		if( !renderer.isRenderable( box, projThresh ) )
		{
			return false;
		}
		
		return true;
	}
	
	template< typename Point >
	inline void Front< Point >::prune( FrontListIter iter, const OctreeDim& nodeLvlDim, const Node& parentNode,
									   Renderer& renderer )
	{
		//cout << "=== Prunning begins ===" << endl << endl;
		
		if( m_releaseFlag )
		{
			releaseSiblings( parentNode.child(), nodeLvlDim );
		}
		
		while( next( iter )->m_octreeNode.parent() == &parentNode )
		{
			m_front.erase_after( iter );
		}
		
		setupNodeRendering( parentNode, renderer );
		
		//cout << "=== Prunning ends ===" << endl << endl;
	}
	
	template< typename Point >
	inline bool Front< Point >::checkBranch( Renderer& renderer, const Float& projThresh, bool& out_isCullable ) const
	{
		pair< Vec3, Vec3 > box = ParentOctree::getBoundaries( code );
		out_isCullable = renderer.isCullable( box );
		
		return !renderer.isRenderable( box, projThresh ) && !out_isCullable;
	}
	
	template< typename Point >
	inline bool Front< Point >::branch( Renderer& renderer )
	{
		//cout << "=== Branching begins ===" << endl << endl;
		
		auto nodeIt = ParentOctree::m_hierarchy->find( code );
		assert( nodeIt != ParentOctree::m_hierarchy->end() );
		OctreeNodePtr node = nodeIt->second;
		
		bool isInner = !node->isLeaf();
		bool childFound = false;
		if( isInner )
		{
			MortonCodePtr firstChild = code->getFirstChild();
			auto childIt = ParentOctree::m_hierarchy->lower_bound( firstChild );
			
			childFound = childIt != ParentOctree::m_hierarchy->end() && *childIt->first->traverseUp() == *code;
			
			if( !childFound )
			{
				childIt = ParentOctree::m_hierarchy->end();
			}
			
			onBranchingItAcquired( childIt, firstChild );
			
			while( childIt != ParentOctree::m_hierarchy->end() && *childIt->first->traverseUp() == *code )
			{
				MortonCodePtr childCode = childIt->first;
				
				//cout << "Into front: " << childCode->getPathToRoot( true ) << endl;
				
				m_frontBehavior->insert( *childCode );
				
				pair< Vec3, Vec3 > box = ParentOctree::getBoundaries( childCode );
				if( !renderer.isCullable( box ) )
				{
					//cout << "Point set to render: " << hex << child->getBits() << dec << endl;
					setupNodeRenderingNoFront( childIt->second, childCode, renderer );
				}
				
				++childIt;
			}
		}
		
		//cout << "Child found? " << boolalpha  << childFound << endl << endl;
		
		if( !childFound )
		{
			//cout << "Children not available. Is leaf? " << boolalpha << !isInner << endl << endl;
			setupNodeRenderingNoFront( node, code, renderer );
		}
		
		//cout << "=== Branching ends ===" << endl << endl;
		
		return childFound;
	}
}

#endif