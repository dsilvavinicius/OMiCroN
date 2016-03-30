#ifndef FRONT_H
#define FRONT_H

#include <list>
#include "SQLiteManager.h"
#include "O1OctreeNode.h"
#include "OctreeDimensions.h"
#include "OctreeStats.h"
#include "RenderingState.h"
#include "Profiler.h"

using namespace std;
using namespace util;

namespace model
{
	// TODO: Probably would be better using list instead of forward_list because of the O(1) splice.
	/** Visualization front of the hierarchy. The front leaf nodes are assumed to be inserted by threads different of the
	 * visualization threads. */
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
		
		/** The node type that is used in front. */
		typedef struct FrontNode
		{
			FrontNode( Node& node, unsigned char lvl )
			: m_octreeNode( node ),
			m_lvl( lvl )
			{}
			
			Node& m_octreeNode;
			unsigned char m_lvl;
		} FrontNode;
		
		using FrontList = list< FrontNode, ManagedAllocator< FrontNode > >;
		using FrontListIter = typename FrontList::iterator;
		using InsertionVector = vector< FrontList, ManagedAllocator< FrontList > >;
		
		/** Ctor.
		 * @param dbFilename is the path to a database file which will be used to store nodes in an out-of-core approach.
		 * @param leafLvlDim is the information of octree size at the deepest (leaf) level. */
		Front( const string& dbFilename, const OctreeDim& leafLvlDim, const int nThreads );
		
		/** Synchronized. Inserts a leaf sibling group into front so it can be tracked later on.
		 * @param leafSiblings is the sibling group array */
		void insertIntoFront( Node& node, unsigned char level, int threadIdx );
		
		/** Notifies that all threads have finished an insertion iteration. */
		void notifyInsertionEnd();
		
		/** Indicates that node release is needed. The front tracking will release nodes until a call to turnReleaseOff()
		 * occurs. */
		void turnReleaseOn();
		
		/** Indicates that node release is not needed anymore. The front tracking will stop the node release. */
		void turnReleaseOff();
		
		/** Notifies that all leaf level nodes are already loaded. */
		void notifyLeafLvlLoaded();
		
		/** Tracks the front based on the projection threshold.
		 * @param renderer is the responsible of rendering the points of the tracked front.
		 * @param projThresh is the projection threashold */
		FrontOctreeStats trackFront( Renderer& renderer, const Float projThresh );
		
	private:
		void trackNode( FrontListIter& frontIt, FrontListIter& beforeFrontEnd, Node*& lastParent, Renderer& renderer,
						const Float projThresh );
		
		bool checkPrune( const Morton& parentMorton, const Node* parentNode, const OctreeDim& parentLvlDim,
						 FrontListIter& frontIt, FrontListIter& beforeFrontEnd, Renderer& renderer,
				   const Float projThresh ) const;
		
		void prune( FrontListIter& frontIt, const OctreeDim& nodeLvlDim, Node* parentNode, Renderer& renderer );
		
		bool checkBranch( const OctreeDim& nodeLvlDim, const Node& node, const Morton& morton, Renderer& renderer,
						  const Float projThresh, bool& out_isCullable ) const;
		
		void branch( FrontListIter& iter, Node& node, const OctreeDim& nodeLvlDim, Renderer& renderer );
		
		/** Persists and release nodes in order to ease memory pressure. */
		void releaseSiblings( NodeArray& siblings, const OctreeDim& dim );
		
		void setupNodeRendering( FrontListIter& iter, const FrontNode& frontNode, Renderer& renderer );
		
		void setupNodeRenderingNoFront( FrontListIter& iter, const Node& node, Renderer& renderer ) const;
		
		/** Database connection used to persist nodes. */
		Sql m_sql;
		
		/** Front internal representation. */
		FrontList m_front;
		
		/** Mutex to synchronize insertions into front with the rendering procedure. */
		mutex m_frontMtx;
		
		/** FrontLists that need to be inserted into front. */
		//InsertionVector m_insertionLists;
		
		/** Insertions done in the current insertion iteration. m_currentIterInsertions[ i ] have the insertions for
		 * thread i. This list is spliced into m_front after notifyInsertionEnd() is called. */
		InsertionVector m_currentIterInsertions;
		
		/** Dimensions of the octree nodes at deepest level. */
		OctreeDim m_leafLvlDim;
		
		/** Number of nodes processed in a frame. */
		ulong m_processedNodes;
		
		/** true if release is on, false otherwise. */
		atomic_bool m_releaseFlag;
		
		/** Indicates that all leaf level nodes are already loaded. */
		atomic_bool m_leafLvlLoadedFlag;
	};
	
	template< typename Morton, typename Point >
	inline Front< Morton, Point >::Front( const string& dbFilename, const OctreeDim& leafLvlDim, const int nThreads )
	: m_sql( dbFilename ),
	m_leafLvlDim( leafLvlDim ),
	m_currentIterInsertions( nThreads ),
	m_processedNodes( 0ul ),
	m_releaseFlag( false ),
	m_leafLvlLoadedFlag( false )
	{}
	
	template< typename Morton, typename Point >
	inline void Front< Morton, Point >::turnReleaseOn()
	{
		// Debug
		{
			cout << "Front release on" << endl << endl;
		}
		
		m_releaseFlag = true;
	}
	
	template< typename Morton, typename Point >
	void Front< Morton, Point >::turnReleaseOff()
	{
		// Debug
		{
			cout << "Front release off" << endl << endl;
		}
		
		m_releaseFlag = false;
	}
	
	template< typename Morton, typename Point >
	void Front< Morton, Point >::notifyLeafLvlLoaded()
	{
		m_leafLvlLoadedFlag = true;
	}
	
	template< typename Morton, typename Point >
	inline void Front< Morton, Point >::insertIntoFront( Node& node, unsigned char level, int threadIdx )
	{
		// Debug
// 		{
// 			cout << "Front insertion: " << OctreeDim( m_leafLvlDim, level ).calcMorton( node ).getPathToRoot( true )
// 				 << endl << endl;
// 		}
		
		FrontList& list = m_currentIterInsertions[ threadIdx ];
		list.push_back( FrontNode( node, level ) );
	}
	
	template< typename Morton, typename Point >
	inline void Front< Morton, Point >::notifyInsertionEnd()
	{
		{
			cout << "Notify insertion end: " << endl << endl;
		}
		
		lock_guard< mutex > lock( m_frontMtx );
		
		for( FrontList list : m_currentIterInsertions )
		{
			m_front.splice( m_front.end(), list );
		}
	}
	
	template< typename Morton, typename Point >
	inline FrontOctreeStats Front< Morton, Point >::trackFront( Renderer& renderer, const Float projThresh )
	{
		m_processedNodes = 0ul;
		auto start = Profiler::now();
		
		bool isFrontEmpty = false;
		// The stop condition is the node before the front end instead of the front end itself. It's because the front
		// end can be changed in parallel by the hierarchy creation thread if nodes are inserted into front. Specifically,
		// the last node cannot be nor prunned since its sibling group can be incomplete nor branched because list
		// insertion requires an iterator to a position after the insertion point.
		FrontListIter beforeFrontEnd; 
		{
			lock_guard< mutex > lock( m_frontMtx );
			isFrontEmpty = m_front.empty();
			
			if( !isFrontEmpty )
			{
				beforeFrontEnd = prev( m_front.end() );
			}
			
			// Debug
			{
				cout << "Tracking front. Size: " << m_front.size() << endl << endl;
			}
		}
		
		if( !isFrontEmpty )
		{
			if( m_releaseFlag )
			{
				m_sql.beginTransaction();
			}
			
			FrontListIter frontIt = m_front.begin();
			Node* lastParent = nullptr; // Parent of last node. Used to optimize prunning check.
			while( frontIt != beforeFrontEnd )
			{
				trackNode( frontIt, beforeFrontEnd, lastParent, renderer, projThresh );
			}
			
			// The last node is always rendered if not culled, since nor prunning nor branching can be applied on it.
			OctreeDim lastNodeDim( m_leafLvlDim, frontIt->m_lvl );
			pair< Vec3, Vec3 > lastNodeBox = lastNodeDim.getNodeBoundaries( frontIt->m_octreeNode );
			if( !renderer.isCullable( lastNodeBox ) )
			{
				renderer.handleNodeRendering( frontIt->m_octreeNode.getContents() );
			}
			
			if( m_releaseFlag )
			{
				m_sql.endTransaction();
			}
		}
		
		int traversalTime = Profiler::elapsedTime( start );
		
		start = Profiler::now();
		
		unsigned int numRenderedPoints = renderer.render();
		
		int renderingTime = Profiler::elapsedTime( start );
		
		return FrontOctreeStats( traversalTime, renderingTime, numRenderedPoints, m_processedNodes );
	}
	
	template< typename Morton, typename Point >
	inline void Front< Morton, Point >::trackNode( FrontListIter& frontIt, FrontListIter& beforeFrontEnd,
												   Node*& lastParent, Renderer& renderer, const Float projThresh )
	{
		FrontNode& frontNode = *frontIt;
		Node& node = frontNode.m_octreeNode;
		OctreeDim nodeLvlDim( m_leafLvlDim, frontNode.m_lvl );
		Morton morton = nodeLvlDim.calcMorton( node );
		Node* parentNode = node.parent();
		
		// Debug
		{
			cout << "Tracking: " << morton.getPathToRoot( true ) << endl << endl;
		}
		
		// If parentNode == lastParent, prunning was not sucessful for a sibling of the current node, so the prunning
		// check can be skipped.
		if( parentNode != nullptr && parentNode != lastParent )  
		{
			OctreeDim parentLvlDim( nodeLvlDim, nodeLvlDim.m_nodeLvl - 1 );
			Morton parentMorton = *morton.traverseUp();
		
			if( checkPrune( parentMorton, parentNode, parentLvlDim, frontIt, beforeFrontEnd, renderer, projThresh ) )
			{
				prune( frontIt, nodeLvlDim, parentNode, renderer );
				return;
			}
		}
		
		bool isCullable = false;
		
		//cout << "Tracking: " << code->getPathToRoot( true ) << endl;
		if( checkBranch( nodeLvlDim, node, morton, renderer, projThresh, isCullable ) )
		{
			//cout << "Branching" << endl << endl;
			branch( frontIt, node, nodeLvlDim, renderer );
			return;
		}
		
		if( !isCullable )
		{
			// No prunning or branching done. Just send the current front node for rendering.
			setupNodeRenderingNoFront( frontIt, node, renderer );
		}
	}
	
	template< typename Morton, typename Point >
	inline bool Front< Morton, Point >::checkPrune( const Morton& parentMorton, const Node* parentNode,
													const OctreeDim& parentLvlDim, FrontListIter& frontIt,
												 FrontListIter& beforeFrontEnd, Renderer& renderer,
												 const Float projThresh ) const
	{
		pair< Vec3, Vec3 > parentBox = parentLvlDim.getMortonBoundaries( parentMorton );
		
		bool pruneFlag = false;
		if( renderer.isCullable( parentBox ) )
		{
			pruneFlag = true;
		}
		else
		{
			if( renderer.isRenderable( parentBox, projThresh ) )
			{
				pruneFlag = true;
			}
		}
		
		if( pruneFlag )
		{
			if( !m_leafLvlLoadedFlag )
			{
				// The last sibling group cannot be prunned until all leaf level nodes are loaded.
				FrontListIter siblingIter = frontIt;
				while( siblingIter != beforeFrontEnd && siblingIter++->m_octreeNode.parent() == parentNode )
				{}
				
				if( siblingIter == beforeFrontEnd && siblingIter->m_octreeNode.parent() == parentNode )
				{
					pruneFlag = false;
				}
			}
		}
		
		return pruneFlag;
	}
	
	template< typename Morton, typename Point >
	inline void Front< Morton, Point >::prune( FrontListIter& frontIt, const OctreeDim& nodeLvlDim, Node* parentNode,
											   Renderer& renderer )
	{
		//cout << "=== Prunning begins ===" << endl << endl;
		
		if( m_releaseFlag )
		{
			releaseSiblings( parentNode->child(), nodeLvlDim );
		}
		
		while( frontIt->m_octreeNode.parent() == parentNode )
		{
			++m_processedNodes;
			frontIt = m_front.erase( frontIt );
		}
		
		FrontNode frontNode( *parentNode, nodeLvlDim.m_nodeLvl + 1 );
		setupNodeRendering( frontIt, frontNode, renderer );
		
		//cout << "=== Prunning ends ===" << endl << endl;
	}
	
	template< typename Morton, typename Point >
	inline bool Front< Morton, Point >::checkBranch( const OctreeDim& nodeLvlDim, const Node& node, const Morton& morton,
													 Renderer& renderer, const Float projThresh, bool& out_isCullable )
	const
	{
		pair< Vec3, Vec3 > box = nodeLvlDim.getMortonBoundaries( morton );
		out_isCullable = renderer.isCullable( box );
		
		if( !node.isLeaf() && !node.child().empty() )
		{
			return !renderer.isRenderable( box, projThresh ) && !out_isCullable;
		}
		
		return false;
	}
	
	// TODO: Verify if there is need for release in branch too.
	template< typename Morton, typename Point >
	inline void Front< Morton, Point >::branch( FrontListIter& frontIt, Node& node, const OctreeDim& nodeLvlDim,
												Renderer& renderer )
	{
		//cout << "=== Branching begins ===" << endl << endl;
		++m_processedNodes;
		frontIt = m_front.erase( frontIt );
		
		OctreeDim childLvlDim( nodeLvlDim, nodeLvlDim.m_nodeLvl + 1 );
		NodeArray& children = node.child();
		
		for( int i = 0; i < children.size(); ++i )
		{
			Node& child = children[ i ];
			pair< Vec3, Vec3 > box = childLvlDim.getNodeBoundaries( child );
			FrontNode frontNode( child, childLvlDim.m_nodeLvl );
			
			if( !renderer.isCullable( box ) )
			{
				//cout << "Point set to render: " << hex << child->getBits() << dec << endl;
				
				setupNodeRendering( frontIt, frontNode, renderer );
			}
			else
			{
				m_front.insert( frontIt, frontNode );
			}
		}
		
		//cout << "=== Branching ends ===" << endl << endl;
	}
	
	template< typename Morton, typename Point >
	inline void Front< Morton, Point >::setupNodeRendering( FrontListIter& frontIt, const FrontNode& frontNode,
															Renderer& renderer )
	{
		m_front.insert( frontIt, frontNode );
		//cout << "Into front: " << code->getPathToRoot( true ) << endl;
		renderer.handleNodeRendering( frontNode.m_octreeNode.getContents() );
	}
	
	template< typename Morton, typename Point >
	inline void Front< Morton, Point >::setupNodeRenderingNoFront( FrontListIter& frontIt, const Node& node,
																   Renderer& renderer ) const
	{
		frontIt++;
		renderer.handleNodeRendering( node.getContents() );
	}
	
	template< typename Morton, typename Point >
	inline void Front< Morton, Point >::releaseSiblings( NodeArray& siblings, const OctreeDim& dim )
	{
		for( int i = 0; i < siblings.size(); ++i )
		{
			Node& sibling = siblings[ i ];
			NodeArray& children = sibling.child();
			if( !children.empty() )
			{
				releaseSiblings( children, OctreeDim( dim, dim.m_nodeLvl + 1 ) );
			}
			
			Morton siblingMorton = dim.calcMorton( sibling );
			// Debug
// 			{
// 				Morton expectedParent; expectedParent.build( 0x41657ff6a9fUL );
// 				Morton expectedA = *expectedParent.getFirstChild();
// 				Morton expectedB = *expectedParent.getLastChild();
// 				
// 				if( expectedA <= siblingMorton && siblingMorton <= expectedB )
// 				{
// 					cout << "2DB: " << hex << siblingMorton.getPathToRoot( true ) << endl;
// 				}
// 			}
			
			// Persisting node
			m_sql.insertNode( siblingMorton, sibling );
		}
		
		siblings.clear();
	}
}

#endif