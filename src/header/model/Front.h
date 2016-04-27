#ifndef FRONT_H
#define FRONT_H

#include <list>
#include "SQLiteManager.h"
#include "O1OctreeNode.h"
#include "OctreeDimensions.h"
#include "OctreeStats.h"
#include "RenderingState.h"
#include "Profiler.h"

#define DEBUG

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
			FrontNode( Node& node, const Morton& morton )
			: m_octreeNode( &node ),
			m_morton( morton )
			{}
			
			FrontNode( const FrontNode& other )
			: m_octreeNode( other.m_octreeNode ),
			m_morton( other.m_morton )
			{}
			
			FrontNode& operator=( const FrontNode& other )
			{
				m_octreeNode = other.m_octreeNode;
				m_morton = other.m_morton;
				return *this;
			}
			
			Node* m_octreeNode;
			Morton m_morton;
		} FrontNode;
		
		using FrontList = list< FrontNode, ManagedAllocator< FrontNode > >;
		using FrontListIter = typename FrontList::iterator;
		using InsertionVector = vector< FrontList, ManagedAllocator< FrontList > >;
		
		/** Ctor.
		 * @param dbFilename is the path to a database file which will be used to store nodes in an out-of-core approach.
		 * @param leafLvlDim is the information of octree size at the deepest (leaf) level. */
		Front( const string& dbFilename, const OctreeDim& leafLvlDim, const int nThreads, const ulong memoryLimit );
		
		/** Synchronized. Inserts a leaf node into front so it can be tracked later on. If a placeholder in a deeper level
		 * for this node is already inserted, it is replaced by the new node.
		 * @param node is a reference to the node to be inserted.
		 * @param morton is node's morton code id.
		 * @param threadIdx is the hierarchy creation thread index of the caller thread. */
		void insertIntoFront( Node& node, const Morton& morton, int threadIdx );
		
		/** Synchronized. Inserts a placeholder for a node that will be defined later in the shallower levels. This node
		 * will be replaced on front tracking if a substitute is already defined.
		 * @param morton is the placeholder node id. */
		void insertPlaceholder( const Morton& morton, int threadIdx );
		
		/** Synchronized. Notifies that all threads have finished an insertion iteration.
		 * @param dispatchedThreads is the number of dispatched thread in the creation iteration. */
		void notifyInsertionEnd( uint dispatchedThreads );
		
		/** Checks if the front is in release mode. */
		bool isReleasing();
		
		/** Notifies that all leaf level nodes are already loaded. */
		void notifyLeafLvlLoaded();
		
		/** Tracks the front based on the projection threshold.
		 * @param renderer is the responsible of rendering the points of the tracked front.
		 * @param projThresh is the projection threashold */
		FrontOctreeStats trackFront( Renderer& renderer, const Float projThresh );
		
		#ifdef DEBUG
			long reportPlaceholders()
			{
				return m_nPlaceholders;
			}
		#endif
		
	private:
		void trackNode( FrontListIter& frontIt, Node*& lastParent, int substitutionLvl, Renderer& renderer,
						const Float projThresh );
		
		/** Substitute a placeholder with the first node of the given substitution level. */
		bool substitutePlaceholder( FrontNode& node, uint substitutionLvl );
		
		bool checkPrune( const Morton& parentMorton, const Node* parentNode, const OctreeDim& parentLvlDim,
						 FrontListIter& frontIt, int substituionLvl, Renderer& renderer, const Float projThresh );
		
		void prune( FrontListIter& frontIt, const OctreeDim& nodeLvlDim, Node* parentNode, Renderer& renderer );
		
		bool checkBranch( const OctreeDim& nodeLvlDim, const Node& node, const Morton& morton, Renderer& renderer,
						  const Float projThresh, bool& out_isCullable ) const;
		
		void branch( FrontListIter& iter, Node& node, const OctreeDim& nodeLvlDim, Renderer& renderer );
		
		/** Persists and release nodes in order to ease memory pressure. */
		void releaseSiblings( NodeArray& siblings, const OctreeDim& dim );
		
		void setupNodeRendering( FrontListIter& iter, const FrontNode& frontNode, Renderer& renderer );
		
		void setupNodeRenderingNoFront( FrontListIter& iter, const Node& node, Renderer& renderer ) const;
		
		#ifdef DEBUG
			void logAndFail( const string& msg )
			{
				logDebugMsg( msg );
				m_log.flush();
				assert( false );
			}
		
			void logDebugMsg( const string& msg )
			{
				lock_guard< recursive_mutex > lock( m_logMutex );
				m_log << msg;
			}
		
			void assertNode( const Node& node, const Morton& morton )
			{
				uint nodeLvl = morton.getLevel();
				OctreeDim nodeDim( m_leafLvlDim, nodeLvl );
				
				stringstream ss;
				ss << "Asserting: " << morton.toString() << " Addr: " << &node << endl << endl;
				
				if( &node == &m_placeholder )
				{
					uint level = morton.getLevel();
					if( level != m_leafLvlDim.m_nodeLvl )
					{
						ss << "Placeholder is not from leaf level." << endl << endl;
						logAndFail( ss.str() );
					}
				}
				else
				{
					if( node.getContents().empty() )
					{
						ss << "Empty node" << endl << endl;
						logAndFail( ss.str() );
					}
					Morton calcMorton = nodeDim.calcMorton( node );
					if( calcMorton != morton )
					{
						ss << "Morton inconsistency. Calc: " << calcMorton.toString() << endl << endl;
						logAndFail( ss.str() );
					}
					
					OctreeDim parentDim( nodeDim, nodeDim.m_nodeLvl - 1 );
					Node* parentNode = node.parent();
					
					if( parentNode != nullptr )
					{
						if( parentNode->getContents().empty() )
						{
							ss << "Empty parent" << endl << endl;
							logAndFail( ss.str() );
						}
						
						Morton parentMorton = *morton.traverseUp();
						Morton calcParentMorton = parentDim.calcMorton( *parentNode );
						if( parentMorton != calcParentMorton )
						{
							ss << "traversal parent: " << parentMorton.toString() << " Calc parent: "
							<< calcParentMorton.toString() << endl;
							logAndFail( ss.str() );
						}
					}
				}
			}
		#endif
		
		/** Database connection used to persist nodes. */
		Sql m_sql;
		
		/** Front internal representation. */
		FrontList m_front;
		
		/** Node used as a placeholder in the front. It is used whenever it is known that a node should occupy a given
		 * position, but the node itself is not defined yet because the hierarchy creation algorithm have not reached
		 * the needed level. */
		Node m_placeholder;
		
		/** Mutexes to synchronize m_perLvlInsertions operations. */
		vector< mutex > m_perLvlMtx;
		
		/** Mutex to sincronize prunning operations with other ones that should have mutual exclusion with
		 * it. */
// 		mutex m_prunningMtx;
		
		/** FrontLists that need to be inserted into front. */
		//InsertionVector m_insertionLists;
		
		/** Nodes pending insertion in the current insertion iteration. m_currentIterInsertions[ t ] have the insertions
		 * for thread t. This list is spliced into m_perLvlInsertions after notifyInsertionEnd() is called. */
		InsertionVector m_currentIterInsertions;
		
		/** All pending insertion nodes. m_perLvlInsertions[ l ] have the nodes for level l. */
		InsertionVector m_perLvlInsertions;
		
		/** Dimensions of the octree nodes at deepest level. */
		OctreeDim m_leafLvlDim;
		
		/** Memory usage limit. Used to turn on/off node release. */
		ulong m_memoryLimit;
		
		/** Number of nodes processed in a frame. */
		ulong m_processedNodes;
		
		/** Number of persisted nodes in the last front tracking operation. */
		ulong m_persisted;
		
		/** true if the front has nodes to release yet, false otherwise. */
		atomic_bool m_releaseFlag;
		
		/** Indicates that all leaf level nodes are already loaded. */
		atomic_bool m_leafLvlLoadedFlag;
		
		#ifdef DEBUG
			recursive_mutex m_logMutex;
			ofstream m_log;
			atomic_long m_nPlaceholders;
		#endif
	};
	
	template< typename Morton, typename Point >
	inline Front< Morton, Point >::Front( const string& dbFilename, const OctreeDim& leafLvlDim, const int nThreads,
										  const ulong memoryLimit )
	: m_sql( dbFilename ),
	m_leafLvlDim( leafLvlDim ),
	m_memoryLimit( memoryLimit ),
	m_currentIterInsertions( nThreads ),
	m_perLvlInsertions( leafLvlDim.m_nodeLvl + 1 ),
	m_perLvlMtx( leafLvlDim.m_nodeLvl + 1 ),
	m_processedNodes( 0ul ),
	m_releaseFlag( false ),
	m_leafLvlLoadedFlag( false )
	#ifdef DEBUG
		,m_log( "Front.txt" ),
		m_nPlaceholders( 0ul )
	#endif
	{}
	
// 	template< typename Morton, typename Point >
// 	inline void Front< Morton, Point >::turnReleaseOn()
// 	{
// 		Debug
// 		{
// 			lock_guard< mutex > lock( m_logMutex );
// 			m_log << "Front release on" << endl << endl;
// 		}
// 		
// 		m_releaseFlag = true;
// 	}
	
// 	template< typename Morton, typename Point >
// 	void Front< Morton, Point >::turnReleaseOff()
// 	{
// 		// Debug
// // 		{
// // 			lock_guard< mutex > lock( m_logMutex );
// // 			m_log << "Front release off" << endl << endl;
// // 		}
// 		
// 		m_releaseFlag = false;
// 	}
	
	template< typename Morton, typename Point >
	void Front< Morton, Point >::notifyLeafLvlLoaded()
	{
		m_leafLvlLoadedFlag = true;
	}
	
	template< typename Morton, typename Point >
	inline bool Front< Morton, Point >::isReleasing()
	{
		return m_releaseFlag;
	}
	
	template< typename Morton, typename Point >
	inline void Front< Morton, Point >::insertIntoFront( Node& node, const Morton& morton, int threadIdx )
	{
		#ifdef DEBUG
		{
			assertNode( node, morton );
		}
		#endif
		
		// Debug
// 		{
// 			lock_guard< recursive_mutex > lock( m_logMutex );
// 			m_log << "Front insertion: " << morton.toString() << " Addr: " << &node << endl << endl;
// 		}
		
		FrontList& list = m_currentIterInsertions[ threadIdx ];
		list.push_back( FrontNode( node, morton ) );
	}
	
	template< typename Morton, typename Point >
	inline void Front< Morton, Point >::insertPlaceholder( const Morton& morton, int threadIdx )
	{
		assert( morton.getLevel() == m_leafLvlDim.m_nodeLvl && "Placeholders should be in hierarchy's deepest level." );
		
		FrontList& list = m_currentIterInsertions[ threadIdx ];
		list.push_back( FrontNode( m_placeholder, morton ) );
	}
	
	template< typename Morton, typename Point >
	inline void Front< Morton, Point >::notifyInsertionEnd( uint dispatchedThreads )
	{
		if( dispatchedThreads > 0 )
		{
			uint lvl = 0;
			for( FrontList& list : m_currentIterInsertions )
			{
				if( !list.empty() )
				{
					lvl = list.front().m_morton.getLevel();
				}
			}
			
			if( lvl )
			{
				lock_guard< mutex > lock( m_perLvlMtx[ lvl ] );
			
				// Debug
// 				{
// 					lock_guard< recursive_mutex > lock( m_logMutex );
// 					m_log << "Notifying insertion end in lvl " << lvl << endl << endl;
// 				}
				
				for( auto it = m_currentIterInsertions.rbegin(); it != m_currentIterInsertions.rend(); ++it )
				{
					m_perLvlInsertions[ lvl ].splice( m_perLvlInsertions[ lvl ].end(), *it );
				}
				
				// Debug
	// 			{
	// 				if( !m_front.empty() )
	// 				{
	// 					lock_guard< mutex > lock( m_logMutex );
	// 					m_log << "Notify insertion end. Fron after insertion:" << endl << endl;
	// 					for( FrontNode& node : m_front )
	// 					{
	// 						m_log << OctreeDim( m_leafLvlDim, node.m_lvl ).calcMorton( node.m_octreeNode ).toString() << endl;
	// 					}
	// 				}
	// 			}
			}
		}
	}
	
// 	template< typename Morton, typename Point >
// 	inline void Front< Morton, Point >::lockPrunning()
// 	{
// 		m_prunningMtx.lock();
// 	}
// 	
// 	template< typename Morton, typename Point >
// 	inline void Front< Morton, Point >::unlockPrunning()
// 	{
// 		m_prunningMtx.unlock();
// 	}
	
	template< typename Morton, typename Point >
	inline FrontOctreeStats Front< Morton, Point >::trackFront( Renderer& renderer, const Float projThresh )
	{
		m_processedNodes = 0ul;
		
		// Debug
// 		if( m_persisted )
// 		{
// 			lock_guard< mutex > lock( m_logMutex );
// 			m_log << "==== TRACKING FRONT ====" << endl << endl;
// 		}
		
		m_persisted = 0ul;
		
		auto start = Profiler::now();
		
		{
			lock_guard< mutex > lock( m_perLvlMtx[ m_leafLvlDim.m_nodeLvl ] );
			// Insert all leaf level pending nodes and placeholders.
			
			#ifdef DEBUG
				m_nPlaceholders += m_perLvlInsertions[ m_leafLvlDim.m_nodeLvl ].size();
			#endif
			
			m_front.splice( m_front.end(), m_perLvlInsertions[ m_leafLvlDim.m_nodeLvl ] );
		}
		
		if( !m_front.empty() )
		{
			// The level from which the placeholders will be substituted is the one with max size, in order to maximize
			// placeholder substitution.
			size_t maxSize = 1;
			int substitutionLvl = 0;
			for( int i = 1; i < m_perLvlInsertions.size() - 1; ++i )
			{
				lock_guard< mutex > lock( m_perLvlMtx[ i ] );
				size_t lvlSize = m_perLvlInsertions[ i ].size();
				maxSize = std::max( maxSize, lvlSize );
				
				if( maxSize == lvlSize )
				{
					substitutionLvl = i;
				}
			}
			
			bool transactionNeeded = AllocStatistics::totalAllocated() > m_memoryLimit;
			m_releaseFlag = transactionNeeded;
			
			if( transactionNeeded )
			{
				m_sql.beginTransaction();
				
				#ifdef DEBUG
// 					stringstream ss; ss << "Front size: " << m_front.size() << endl << endl;
// 					logDebugMsg( ss.str() );
				#endif
			}
			
			// Debug
// 			{
// 				lock_guard< recursive_mutex > lock( m_logMutex );
// 				m_log << "===== Front tracking starts =====." << endl << endl;
// 			}
			
			Node* lastParent = nullptr; // Parent of last node. Used to optimize prunning check.
			for( FrontListIter frontIt = m_front.begin(); frontIt != m_front.end(); /**/ )
			{
				#ifdef DEBUG
				{
					assertNode( *frontIt->m_octreeNode, frontIt->m_morton );
				}
				#endif
				trackNode( frontIt, lastParent, substitutionLvl, renderer, projThresh );
			}
			
			if( transactionNeeded )
			{
				m_sql.endTransaction();
			}
		}
		
		#ifdef DEBUG
			stringstream ss; ss << "Front size: " << m_front.size() << " Placeholders: " << m_nPlaceholders
								<< " Persisted: " << m_persisted << endl << endl;
			logDebugMsg( ss.str() );
		#endif
		
		if( m_releaseFlag && m_persisted == 0 )
		{
			#ifdef DEBUG
				ss << "No more nodes can be persisted." << endl << endl;
				logDebugMsg( ss.str() );
			#endif
			
			m_releaseFlag = false;
		}
		
		int traversalTime = Profiler::elapsedTime( start );
		
		start = Profiler::now();
		
		unsigned int numRenderedPoints = renderer.render();
		
		int renderingTime = Profiler::elapsedTime( start );
		
		return FrontOctreeStats( traversalTime, renderingTime, numRenderedPoints, m_processedNodes );
	}
	
	template< typename Morton, typename Point >
	inline void Front< Morton, Point >::trackNode( FrontListIter& frontIt, Node*& lastParent, int substitutionLvl,
												   Renderer& renderer, const Float projThresh )
	{
		FrontNode& frontNode = *frontIt;
		
		if( frontNode.m_octreeNode == &m_placeholder )
		{
			if( !substitutePlaceholder( frontNode, substitutionLvl ) )
			{
				frontIt++;
				return;
			}
		}
		
		Node& node = *frontNode.m_octreeNode;
		Morton& morton = frontNode.m_morton;
		OctreeDim nodeLvlDim( m_leafLvlDim, morton.getLevel() );
		
		
// 		m_prunningMtx.lock();
		Node* parentNode = node.parent();
// 		m_prunningMtx.unlock();
		
		#ifdef DEBUG
// 		{
// 			stringstream ss; ss << "Tracking: " << morton.toString() << endl << endl;
// 			logDebugMsg( ss.str() );
// 		}
		#endif
		
		// If parentNode == lastParent, prunning was not sucessful for a sibling of the current node, so the prunning
		// check can be skipped.
		if( parentNode != nullptr && parentNode != lastParent )  
		{
			OctreeDim parentLvlDim( nodeLvlDim, nodeLvlDim.m_nodeLvl - 1 );
			Morton parentMorton = *morton.traverseUp();
			
			#ifdef DEBUG
			{
				assertNode( node, morton );
			}
			#endif
			
			if( checkPrune( parentMorton, parentNode, parentLvlDim, frontIt, substitutionLvl, renderer, projThresh ) )
			{
				#ifdef DEBUG
// 				{
// 					stringstream ss; ss << "Prunning: " << morton.toString() << " Parent: " << parentMorton.toString()
// 										<< endl << endl;
// 					logDebugMsg( ss.str() );
// 				}
				#endif
				
				prune( frontIt, nodeLvlDim, parentNode, renderer );
				lastParent = parentNode;
				
				#ifdef DEBUG
				{
					auto tempIt = prev( frontIt );
					assertNode( *tempIt->m_octreeNode, tempIt->m_morton );
				}
				#endif
				
				return;
			}
			lastParent = parentNode;
		}
		
		bool isCullable = false;
		
		if( checkBranch( nodeLvlDim, node, morton, renderer, projThresh, isCullable ) )
		{
			branch( frontIt, node, nodeLvlDim, renderer );
			return;
		}
		
		if( !isCullable )
		{
			// No prunning or branching done. Just send the current front node for rendering.
			setupNodeRenderingNoFront( frontIt, node, renderer );
			return;
		}
		
		frontIt++;
	}
	
	template< typename Morton, typename Point >
	inline bool Front< Morton, Point >::substitutePlaceholder( FrontNode& node, uint substitutionLvl )
	{
		assert( node.m_octreeNode == &m_placeholder && "Substitution paramenter should be a placeholder node" );
		
		if( substitutionLvl != 0 )
		{
			lock_guard< mutex > lock( m_perLvlMtx[ substitutionLvl ] );
			FrontList& substitutionLvlList = m_perLvlInsertions[ substitutionLvl ];
			FrontNode& substituteCandidate = substitutionLvlList.front();
			
			if( node.m_morton.isDescendantOf( substituteCandidate.m_morton ) )
			{
				#ifdef DEBUG
				{
					assertNode( *substituteCandidate.m_octreeNode, substituteCandidate.m_morton );
					--m_nPlaceholders;
				}
				#endif
				
				node = substituteCandidate;
				
				#ifdef DEBUG
				{
					assertNode( *node.m_octreeNode, node.m_morton );
				}
				#endif
				
				substitutionLvlList.erase( substitutionLvlList.begin() );
				
				#ifdef DEBUG
				{
					assertNode( *node.m_octreeNode, node.m_morton );
				}
				#endif
				
				return true;
			}
			else 
			{
				return false;
			}
		}
		
		return false;
	}
	
	template< typename Morton, typename Point >
	inline bool Front< Morton, Point >::checkPrune( const Morton& parentMorton, const Node* parentNode,
													const OctreeDim& parentLvlDim, FrontListIter& frontIt,
												 int substitutionLvl, Renderer& renderer, const Float projThresh )
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
			int nSiblings = 0;
			FrontListIter siblingIter = frontIt;
			while( siblingIter != m_front.end() )
			{
				if( siblingIter->m_octreeNode == &m_placeholder )
				{
					substitutePlaceholder( *siblingIter, substitutionLvl );
				}
				if( siblingIter++->m_octreeNode->parent() != parentNode )
				{
					break;
				}
				++nSiblings;
			}
			
			// The last sibling group cannot be prunned when the leaf level is not loaded yet. It can be incomplete yet
			// at that time. Also, all the sibling nodes should be in front before prunning.
			if( ( !m_leafLvlLoadedFlag && siblingIter == m_front.end() ) || nSiblings != parentNode->child().size() )
			{
				pruneFlag = false;
			}
		}
		
		return pruneFlag;
	}
	
	template< typename Morton, typename Point >
	inline void Front< Morton, Point >::prune( FrontListIter& frontIt, const OctreeDim& nodeLvlDim, Node* parentNode,
											   Renderer& renderer )
	{
		Morton parentMorton = *frontIt->m_morton.traverseUp();
		
		#ifdef DEBUG
			int toRelease = parentNode->child().size();
			ulong processedBefore = m_processedNodes;
			bool released = false;
			
			assertNode( *frontIt->m_octreeNode, frontIt->m_morton );
			
			stringstream ss; ss << "Prunning group of " << frontIt->m_morton.toString() << endl << endl;
			logDebugMsg( ss.str() );
		#endif
		
		while( frontIt != m_front.end() && frontIt->m_octreeNode->parent() == parentNode )
		{
			++m_processedNodes;
			
			#ifdef DEBUG
// 			{
// 				stringstream ss; ss << "Releasing " << frontIt->m_morton.getPathToRoot( true ) << endl;
// 				logDebugMsg( ss.str() );
// 			}
			#endif
			
			frontIt = m_front.erase( frontIt );
		}
		
		if( m_releaseFlag )
		{
			#ifdef DEBUG
				stringstream ss; ss << "Should release" << endl << endl;
				logDebugMsg( ss.str() );
			#endif
			
			if( AllocStatistics::totalAllocated() > m_memoryLimit )
			{
				#ifdef DEBUG
					released = true;
				#endif
				
				releaseSiblings( parentNode->child(), nodeLvlDim );
			}
			else
			{
				m_releaseFlag = false;
			}
		}
		
		#ifdef DEBUG
		{
			if( released && toRelease != m_processedNodes - processedBefore )
			{
				stringstream ss; ss << m_front.front().m_morton.getPathToRoot( true ) << " expected parent: "
									<< parentNode << " found: " << m_front.front().m_octreeNode->parent() << endl << endl;
				logAndFail( ss.str() );
				assert( false && "All persisted nodes should also be released." );
			}
		}
		#endif
		
		FrontNode frontNode( *parentNode, parentMorton );
		
		#ifdef DEBUG
		{
			assertNode( *frontNode.m_octreeNode, frontNode.m_morton );
		}
		#endif
		
		setupNodeRendering( frontIt, frontNode, renderer );
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
		++m_processedNodes;
		frontIt = m_front.erase( frontIt );
		
		OctreeDim childLvlDim( nodeLvlDim, nodeLvlDim.m_nodeLvl + 1 );
		NodeArray& children = node.child();
		
		for( int i = 0; i < children.size(); ++i )
		{
			Node& child = children[ i ];
			pair< Vec3, Vec3 > box = childLvlDim.getNodeBoundaries( child );
			FrontNode frontNode( child, childLvlDim.calcMorton( child ) );
			
			assert( frontNode.m_morton.getBits() != 1 && "Inserting root node into front (branch)." );
			
			if( !renderer.isCullable( box ) )
			{
				setupNodeRendering( frontIt, frontNode, renderer );
			}
			else
			{
				m_front.insert( frontIt, frontNode );
			}
		}
	}
	
	template< typename Morton, typename Point >
	inline void Front< Morton, Point >::setupNodeRendering( FrontListIter& frontIt, const FrontNode& frontNode,
															Renderer& renderer )
	{
		m_front.insert( frontIt, frontNode );
		renderer.handleNodeRendering( frontNode.m_octreeNode->getContents() );
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
			
			#ifdef DEBUG
				stringstream ss; ss << "2DB: " << siblingMorton.getPathToRoot( true ) << endl;
				logDebugMsg( ss.str() );
			#endif
			
			// Persisting node
			m_sql.insertNode( siblingMorton, sibling );
		}
		
		m_persisted += siblings.size();
		
		siblings.clear();
	}
}

#undef DEBUG

#endif