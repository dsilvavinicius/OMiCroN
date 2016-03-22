#ifndef FRONT_H
#define FRONT_H

#include <forward_list>
#include "O1OctreeNode.h"
#include "OctreeDimensions.h"
#include "OctreeStats.h"
#include "RenderingState.h"
#include "Profiler.h"

using namespace std;
using namespace util;

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
		
		/** The node type that is used in front. */
		typedef struct FrontNode
		{
			FrontNode( const Node& node, unsigned char lvl )
			: m_octreeNode( node ),
			m_lvl( lvl )
			{}
			
			Node& m_octreeNode;
			unsigned char m_lvl;
		} FrontNode;
		
		using FrontList = forward_list< FrontNode, ManagedAllocator< FrontNode > >;
		using FrontListIter = typename FrontList::iterator;
		
		/** Ctor.
		 * @param sortedPlyFilename is the path to a .ply file with points sorted in morton code order.
		 * @param leafLvlDim is the information of octree size at the deepest (leaf) level. */
		Front( const string& sortedPlyFilename, const OctreeDim& leafLvlDim );
		
		/** Synchronized. Inserts a leaf sibling group into front so it can be tracked later on.
		 * @param leafSiblings is the sibling group array */
		void insertIntoFront( NodeArray& leafSiblings );
		
		/** Indicates that node release is needed. The front tracking will release nodes until a call to turnReleaseOff()
		 * occurs. */
		void turnReleaseOn();
		
		/** Indicates that node release is not needed anymore. The front tracking will stop the node release. */
		void turnReleaseOff();
		
		/** Notifies that all leaf level nodes are already loaded. */
		void notifyLeafLvlLoaded();
		
		/** Persists and release nodes in order to ease memory pressure. */
		void releaseSiblings( NodeArray& siblings, const OctreeDim& dim );
		
		/** Tracks the front based on the projection threshold.
		 * @param renderer is the responsible of rendering the points of the tracked front.
		 * @param projThresh is the projection threashold */
		FrontOctreeStats trackFront( Renderer& renderer, const Float projThresh );
		
	private:
		void trackNode( FrontListIter& frontIt, FrontListIter& frontEnd, Renderer& renderer, const Float projThresh );
		
		bool checkPrune( const Morton& parentMorton, const Node* parentNode, const OctreeDim& parentLvlDim,
						 FrontListIter& frontIt, FrontListIter& frontEnd, Renderer& renderer, const Float projThresh )
		const;
		
		void prune( FrontListIter& frontIt, const OctreeDim& nodeLvlDim, const Node* parentNode, Renderer& renderer );
		
		bool checkBranch( const OctreeDim& nodeLvlDim, const Node& node, const Morton& morton, Renderer& renderer,
						  const Float projThresh, bool& out_isCullable ) const;
		
		void branch( FrontListIter& iter, const Node& node, const OctreeDim& nodeLvlDim, Renderer& renderer );
		
		void setupNodeRendering( FrontListIter& iter, const FrontNode& frontNode, Renderer& renderer );
		
		void setupNodeRenderingNoFront( FrontListIter& iter, const Node& node, Renderer& renderer ) const;
		
		/** Database connection used to persist nodes. */
		Sql m_sql;
		
		/** Front internal representation. */
		FrontList m_front;
		
		/** Internal front insertion iterator. */
		FrontListIter m_insertionIt;
		
		/** Mutex to synchronize insertions into front from other threads. */
		mutex m_frontMtx;
		
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
	inline Front< Morton, Point >::Front( const string& sortedPlyFilename, const OctreeDim& leafLvlDim )
	: m_sql( sortedPlyFilename ),
	m_leafLvlDim( leafLvlDim ),
	m_front(),
	m_insertionIt( m_front.before_begin() ),
	m_processedNodes( 0ul ),
	m_releaseFlag( false ),
	m_leafLvlLoadedFlag( false )
	{}
	
	template< typename Morton, typename Point >
	inline void Front< Morton, Point >::turnReleaseOn()
	{
		m_releaseFlag = true;
	}
	
	template< typename Morton, typename Point >
	void Front< Morton, Point >::turnReleaseOff()
	{
		m_releaseFlag = false;
	}
	
	template< typename Morton, typename Point >
	void Front< Morton, Point >::notifyLeafLvlLoaded()
	{
		m_leafLvlLoadedFlag = true;
	}
	
	template< typename Morton, typename Point >
	void Front< Morton, Point >::insertIntoFront( NodeArray& leafSiblings )
	{
		for( int i = 0; i < leafSiblings.size(); ++i )
		{
			FrontNode frontNode( leafSiblings[ i ], m_leafLvlDim.m_nodeLvl );
			{
				lock_guard< mutex > lock( m_frontMtx );
				m_insertionIt = m_front.insert_after( m_insertionIt, frontNode );
			}
		}
	}
	
	template< typename Morton, typename Point >
	inline FrontOctreeStats Front< Morton, Point >::trackFront( Renderer& renderer, const Float projThresh )
	{
		m_processedNodes = 0ul;
		auto start = Profiler::now();
		
		if( m_releaseFlag )
		{
			m_sql.beginTransaction();
		}
		
		FrontListIter frontEnd;
		{
			lock_guard< mutex > lock( m_frontMtx );
			frontEnd = m_front.end();
		}
		
		for( FrontListIter frontIt = m_front.before_begin(); next( frontIt ) != frontEnd;/* */)
		{
			trackNode( frontIt, frontEnd, renderer, projThresh );
		}
		
		if( m_releaseFlag )
		{
			m_sql.endTransaction();
		}
		
		int traversalTime = Profiler::elapsedTime( start );
		
		start = Profiler::now();
		
		unsigned int numRenderedPoints = renderer.render();
		
		int renderingTime = Profiler::elapsedTime( start );
		
		return FrontOctreeStats( traversalTime, renderingTime, numRenderedPoints, m_processedNodes );
	}
	
	template< typename Morton, typename Point >
	inline void Front< Morton, Point >::trackNode( FrontListIter& frontIt, FrontListIter& frontEnd, Renderer& renderer,
												   const Float projThresh )
	{
		FrontNode& frontNode = *next( frontIt );
		Node& node = frontNode.m_octreeNode;
		OctreeDim nodeLvlDim( m_leafLvlDim, frontNode.m_lvl );
		Morton morton = nodeLvlDim.calcMorton( node );
		Node* parentNode = node.parent();
		
		if( parentNode != nullptr )
		{
			OctreeDim parentLvlDim( nodeLvlDim, nodeLvlDim.m_lvl - 1 );
			Morton parentMorton = *morton.traverseUp();
		
			if( checkPrune( parentMorton, parentNode, parentLvlDim, frontIt, frontEnd, renderer, projThresh ) )
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
												 FrontListIter& frontEnd, Renderer& renderer, const Float projThresh )
	const
	{
		pair< Vec3, Vec3 > parentBox = parentLvlDim.getBoundaries( parentMorton );
		
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
				FrontListIter siblingIter = next( frontIt );
				while( siblingIter != frontEnd && siblingIter++.parent() == parentNode )
				{}
				
				pruneFlag = siblingIter != frontEnd;
			}
		}
		
		return pruneFlag;
	}
	
	template< typename Morton, typename Point >
	inline void Front< Morton, Point >::prune( FrontListIter& frontIt, const OctreeDim& nodeLvlDim, const Node* parentNode,
											   Renderer& renderer )
	{
		//cout << "=== Prunning begins ===" << endl << endl;
		
		if( m_releaseFlag )
		{
			releaseSiblings( parentNode->child(), nodeLvlDim );
		}
		
		while( next( frontIt )->m_octreeNode.parent() == parentNode )
		{
			++m_processedNodes;
			m_front.erase_after( frontIt );
		}
		
		FrontNode frontNode( *parentNode, nodeLvlDim.m_nodelLvl + 1 );
		setupNodeRendering( frontIt, frontNode, renderer );
		
		//cout << "=== Prunning ends ===" << endl << endl;
	}
	
	template< typename Morton, typename Point >
	inline bool Front< Morton, Point >::checkBranch( const OctreeDim& nodeLvlDim, const Node& node, const Morton& morton,
													 Renderer& renderer, const Float projThresh, bool& out_isCullable )
	const
	{
		pair< Vec3, Vec3 > box = nodeLvlDim.getBoundaries( morton );
		out_isCullable = renderer.isCullable( box );
		
		if( node.isInner() && !node.child().empty() )
		{
			return !renderer.isRenderable( box, projThresh ) && !out_isCullable;
		}
		
		return false;
	}
	
	// TODO: Verify if there is need for release in branch too.
	template< typename Morton, typename Point >
	inline void Front< Morton, Point >::branch( FrontListIter& frontIt, const Node& node, const OctreeDim& nodeLvlDim,
												Renderer& renderer )
	{
		//cout << "=== Branching begins ===" << endl << endl;
		++m_processedNodes;
		m_front.erase_after( frontIt );
		
		OctreeDim childLvlDim( nodeLvlDim, nodeLvlDim.m_nodeLvl + 1 );
		NodeArray& children = node.child();
		
		for( int i = 0; i < children.size(); ++i )
		{
			Node& child = children[ i ];
			pair< Vec3, Vec3 > box = childLvlDim.getBoundaries( child );
			FrontNode frontNode( child, childLvlDim.m_nodeLvl );
			
			if( !renderer.isCullable( box ) )
			{
				//cout << "Point set to render: " << hex << child->getBits() << dec << endl;
				
				setupNodeRendering( frontIt, frontNode, renderer );
			}
			else
			{
				frontIt = m_front->insert_after( frontIt, frontNode );
			}
		}
		
		//cout << "=== Branching ends ===" << endl << endl;
	}
	
	template< typename Morton, typename Point >
	inline void Front< Morton, Point >::setupNodeRendering( FrontListIter& frontIt, const FrontNode& frontNode,
															Renderer& renderer )
	{
		frontIt = m_front.insert_after( frontIt, frontNode );
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