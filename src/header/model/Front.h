#ifndef FRONT_H
#define FRONT_H

#include <list>
#include "SQLiteManager.h"
#include "O1OctreeNode.h"
#include "OctreeDimensions.h"
#include "OctreeStats.h"
#include "RenderingState.h"
#include "Profiler.h"
#include <StackTrace.h>

#define DEBUG

#ifdef DEBUG
	#include "HierarchyCreationLog.h"
#endif

using namespace std;
using namespace util;

namespace model
{
	/** Out-of-core visualization front of an hierarchy under construction. Front ensures that its nodes are always sorted
	 * in hierarchy's width and that nodes can be inserted in a multithreaded environment. The nodes must be inserted in
	 * iterations. For a given iteration, the insertion threads can in parallel insert nodes of a continuous front segment,
	 * given that these segments are disjoint and that all nodes inserted by all threads also form a continuous segment.
	 * Also, a given thread should insert nodes in hierarchy's width-order. In order to ensure this ordering, an API for
	 * defining node placeholders is also available. Placeholders are temporary nodes in the deepest hierarchy level that
	 * are expected to be substituted by meaninful nodes later on, when they are constructed in the hierarchy. All
	 * insertions go to thread buffers untill notifyInsertionEnd is called, which indicates the iteration ending and
	 * results in all inserted nodes being pushed into the front data structure itself.
	 *
	 * Front also provides API for front tracking, operation which prunes or branches front nodes in order to enforce a
	 * rendering performance budget, specified by a box projection threshold. This operation also manages memory stress
	 * by persisting and releasing prunned sibling groups. */
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
			{
				#ifdef DEBUG
				{
					stringstream ss; ss << this << endl;
					HierarchyCreationLog::logDebugMsg( ss.str() );
					StackTrace::log();
				}
				#endif
			}
			
			FrontNode( const FrontNode& other )
			: m_octreeNode( other.m_octreeNode ),
			m_morton( other.m_morton )
			{
				#ifdef DEBUG
				{
					stringstream ss; ss << this << endl;
					HierarchyCreationLog::logDebugMsg( ss.str() );
					StackTrace::log();
				}
				#endif
			}
			
			FrontNode& operator=( const FrontNode& other )
			{
				m_octreeNode = other.m_octreeNode;
				m_morton = other.m_morton;
				
				#ifdef DEBUG
				if( other.m_octreeNode == nullptr )
				{
					stringstream ss; ss << "Front node with null octree node. Addr: " << this << endl << endl;
					throw logic_error( ss.str() );
				}
				if( other.m_morton.getBits() == 0x0 )
				{
					stringstream ss; ss << "Front node with null morton code. Addr: " << this << endl << endl;
					throw logic_error( ss.str() );
				}
				
				stringstream ss; ss << this << endl;
				HierarchyCreationLog::logDebugMsg( ss.str() );
				StackTrace::log();
				
				//other.assertNode();
				//assertNode();
				#endif
				
				return *this;
			}
			
			#ifdef DEBUG
// 			void assertNode() const
// 			{
// 				//stringstream ss;
// 				if( m_octreeNode == nullptr )
// 				{
// 					//ss << "Front node with null octree node. Addr: " << this << endl << endl;
// 					throw logic_error( "Front node with null octree node" );
// 				}
// 				if( m_morton.getBits() == 0x0 )
// 				{
// 					//ss << "Front node with null morton code. Addr: " << this << endl << endl;
// 					throw logic_error( "Front node with null morton code." );
// 				}
// 			}
			#endif
			
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
		
		/** Inserts a node into thread's buffer end so it can be push to the front later on. After this, the tracking
		 * method will ensure that the placeholder related with this node, if any, will be substituted by it.
		 * @param node is a reference to the node to be inserted.
		 * @param morton is node's morton code id.
		 * @param threadIdx is the hierarchy creation thread index of the caller thread. */
		void insertIntoBufferEnd( Node& node, const Morton& morton, int threadIdx );
		
		/** Acquire the iterator to the beginning of a thread buffer.
		 * @param node is a reference to the node to be inserted.
		 * @param morton is node's morton code id.
		 * @param threadIdx is the hierarchy creation thread index of the caller thread.
		 * @returns an iterator pointing to the element inserted into front. */
		FrontListIter getIteratorToBufferBegin( int threadIdx );
		
		/** Inserts a node into thread's buffer, before iter, so it can be push to the front later on. After
		 * this, the tracking method will ensure that the placeholder related with this node, if any, will be substituted
		 * by it.
		 * @param iter is an iterator to a position in the thread's buffer returned by Front's API.
		 * @param node is a reference to the node to be inserted.
		 * @param morton is node's morton code id.
		 * @param threadIdx is the hierarchy creation thread index of the caller thread.
		 * @returns an iterator pointing to the element inserted into front. */
		void insertIntoBuffer( FrontListIter& iter, Node& node, const Morton& morton, int threadIdx );
		
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
// 			void assertFrontIterator( const FrontListIter& iter )
// 			{
// 				if( iter != m_front.begin() )
// 				{
// 					Morton& currMorton = iter->m_morton;
// 					Morton& prevMorton = prev( iter )->m_morton;
// 					
// 					uint currLvl = currMorton.getLevel();
// 					uint prevLvl = prevMorton.getLevel();
// 					
// 					bool isPlaceholder = ( iter->m_octreeNode == &m_placeholder );
// 					stringstream ss;
// 					if( currLvl < prevLvl )
// 					{
// 						Morton prevAncestorMorton = prevMorton.getAncestorInLvl( currLvl );
// 						if( currMorton <= prevAncestorMorton )
// 						{
// 							ss  << "Front order compromised. Prev: " << prevAncestorMorton.getPathToRoot( true )
// 								<< "Curr: " << currMorton.getPathToRoot( true ) << " is placeholder? "
// 								<< isPlaceholder << endl;
// 							HierarchyCreationLog::logAndFail( ss.str() );
// 						}
// 					}
// 					else if( currLvl > prevLvl )
// 					{
// 						Morton currAncestorMorton = currMorton.getAncestorInLvl( prevLvl );
// 						if( currAncestorMorton <= prevMorton  )
// 						{
// 							ss  << "Front order compromised. Prev: " << prevMorton.getPathToRoot( true )
// 								<< "Curr: " << currAncestorMorton.getPathToRoot( true ) << " is placeholder? "
// 								<< isPlaceholder << endl;
// 							HierarchyCreationLog::logAndFail( ss.str() );
// 						}
// 					}
// 					else
// 					{
// 						if( currMorton <= prevMorton )
// 						{
// 							ss  << "Front order compromised. Prev: " << prevMorton.getPathToRoot( true )
// 								<< "Curr: " << currMorton.getPathToRoot( true ) << " is placeholder? "
// 								<< isPlaceholder << endl;
// 							HierarchyCreationLog::logAndFail( ss.str() );
// 						}
// 					}
// 				}
// 				assertNode( *iter->m_octreeNode, iter->m_morton );
// 			}
// 		
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
						HierarchyCreationLog::logAndFail( ss.str() );
					}
				}
				else
				{
					if( node.getContents().empty() )
					{
						ss << "Empty node" << endl << endl;
						HierarchyCreationLog::logAndFail( ss.str() );
					}
					Morton calcMorton = nodeDim.calcMorton( node );
					if( calcMorton != morton )
					{
						ss << "Morton inconsistency. Calc: " << calcMorton.toString() << endl << endl;
						HierarchyCreationLog::logAndFail( ss.str() );
					}
					
					OctreeDim parentDim( nodeDim, nodeDim.m_nodeLvl - 1 );
					Node* parentNode = node.parent();
					
					if( parentNode != nullptr )
					{
						if( parentNode->getContents().empty() )
						{
							ss << "Empty parent" << endl << endl;
							HierarchyCreationLog::logAndFail( ss.str() );
						}
						
						Morton parentMorton = *morton.traverseUp();
						Morton calcParentMorton = parentDim.calcMorton( *parentNode );
						if( parentMorton != calcParentMorton )
						{
							ss << "traversal parent: " << parentMorton.toString() << " Calc parent: "
							<< calcParentMorton.toString() << endl;
							HierarchyCreationLog::logAndFail( ss.str() );
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
		 * of thread t. This lists are moved to m_perLvlInsertions whenever notifyInsertionEnd() is called. */
		InsertionVector m_currentIterInsertions;
		
		/** All pending insertion nodes. m_perLvlInsertions[ l ] have the nodes for level l, sorted in hierarchy width order. */
		InsertionVector m_perLvlInsertions;
		
		/** All placeholders pending insertion in the current insertion iteration. m_currentIterPlaceholders[ t ] have the
		 * insertions of thread t. The lists are moved to m_placeholders whenever notifyInsertionEnd() is called. */
		InsertionVector m_currentIterPlaceholders;
		
		/** All placeholders pending insertion. The list is sorted in hierarchy width order.  */
		FrontList m_placeholders;
		
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
			ulong m_nPlaceholders;
			ulong m_nSubstituted;
		#endif
	};
	
	template< typename Morton, typename Point >
	inline Front< Morton, Point >::Front( const string& dbFilename, const OctreeDim& leafLvlDim, const int nThreads,
										  const ulong memoryLimit )
	: m_sql( dbFilename ),
	m_leafLvlDim( leafLvlDim ),
	m_memoryLimit( memoryLimit ),
	m_currentIterInsertions( nThreads ),
	m_currentIterPlaceholders( nThreads ),
	m_perLvlInsertions( leafLvlDim.m_nodeLvl + 1 ),
	m_perLvlMtx( leafLvlDim.m_nodeLvl + 1 ),
	m_processedNodes( 0ul ),
	m_releaseFlag( false ),
	m_leafLvlLoadedFlag( false )
	#ifdef DEBUG
		, m_nPlaceholders( 0ul ),
		m_nSubstituted( 0ul )
	#endif
	{}

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
	inline void Front< Morton, Point >::insertIntoBufferEnd( Node& node, const Morton& morton, int threadIdx )
	{
		
		#ifdef DEBUG
		{
// 			stringstream ss; ss << "Buffer end insertion: " << morton.getPathToRoot( true ) << endl;
// 			HierarchyCreationLog::logDebugMsg( ss.str() );
			
			assertNode( node, morton );
		}
		#endif
		
		FrontList& list = m_currentIterInsertions[ threadIdx ];
		list.push_back( FrontNode( node, morton ) );
	}
	
	template< typename Morton, typename Point >
	inline typename Front< Morton, Point >::FrontListIter Front< Morton, Point >::getIteratorToBufferBegin( int threadIdx )
	{
		return m_currentIterInsertions[ threadIdx ].begin();
	}
	
	template< typename Morton, typename Point >
	inline void Front< Morton, Point > ::insertIntoBuffer( FrontListIter& iter, Node& node, const Morton& morton,
														   int threadIdx )
	{
		#ifdef DEBUG
		{
// 			stringstream ss; ss << "Buffer iter insertion: " << morton.getPathToRoot( true ) << endl;
// 			HierarchyCreationLog::logDebugMsg( ss.str() );
			
			assertNode( node, morton );
		}
		#endif
		
		FrontList& list = m_currentIterInsertions[ threadIdx ];
		list.insert( iter, FrontNode( node, morton ) );
	}
	
	template< typename Morton, typename Point >
	inline void Front< Morton, Point >::insertPlaceholder( const Morton& morton, int threadIdx )
	{
		assert( morton.getLevel() == m_leafLvlDim.m_nodeLvl && "Placeholders should be in hierarchy's deepest level." );
		
		#ifdef DEBUG
		{
// 			stringstream ss; ss << "Placeholder insertion: " << morton.getPathToRoot( true ) << endl;
// 			if( !m_currentIterPlaceholders[ threadIdx ].empty()
// 				&& morton <= m_currentIterPlaceholders[ threadIdx ].back().m_morton )
// 			{
// 				ss << "Placeholder insertion compromises ordering" << endl << endl;
// 				HierarchyCreationLog::logAndFail( ss.str() );
// 			}
		}
		#endif
		
		FrontList& list = m_currentIterPlaceholders[ threadIdx ];
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
					break;
				}
			}
			
			if( lvl )
			{
				lock_guard< mutex > lock( m_perLvlMtx[ lvl ] );
			
				#ifdef DEBUG
					ulong insertionSize = 0ul;
				#endif
				
				for( auto it = m_currentIterInsertions.begin(); it != m_currentIterInsertions.end(); ++it )
				{
					// Move nodes to the per-level sorted buffer.
					#ifdef DEBUG
// 						insertionSize += it->size();
						
// 						for( FrontNode& node : *it )
// 						{
// 							node.assertNode();
// 						}
					#endif
						
					m_perLvlInsertions[ lvl ].splice( m_perLvlInsertions[ lvl ].end(), *it );
				}
				
				#ifdef DEBUG
// 					stringstream ss; ss << "Notifying insertion end in lvl " << lvl << ". Nodes: " << insertionSize;
// 					ulong nPlaceholders = 0ul;
				#endif
			}
			
			{
				lock_guard< mutex > lock( m_perLvlMtx[ m_leafLvlDim.m_nodeLvl ] );
				
				// Move placeholders to the sorted buffer.
				for( FrontList& list : m_currentIterPlaceholders )
				{
					#ifdef DEBUG
// 					{
// // 						nPlaceholders += list.size();
// 						if( !m_placeholders.empty() && !list.empty() )
// 						{
// 							Morton& prev = m_placeholders.back().m_morton;
// 							Morton& next = list.front().m_morton;
// 							
// 							if( next <= prev )
// 							{
// 								stringstream ss; ss << "Placeholder ordering compromised when moving thread insertions. Prev:"
// 									<< prev.getPathToRoot( true ) << "Next: " << next.getPathToRoot( true );
// 								HierarchyCreationLog::logAndFail( ss.str() );
// 							}
// 						}
// 					}
					#endif
					
					m_placeholders.splice( m_placeholders.end(), list );
				}
			}
			
			#ifdef DEBUG
// 			{
// 				ss << " Placeholders: " << nPlaceholders << endl << endl;
// 				HierarchyCreationLog::logDebugMsg( ss.str() );
// 			}
			#endif
		}
	}
	
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
		
		#ifdef DEBUG
// 			m_nPlaceholders = 0ul;
// 			m_nSubstituted = 0ul;
// 			ulong insertedPlaceholders = 0ul;
		#endif
		
		auto start = Profiler::now();
		
		{
			lock_guard< mutex > lock( m_perLvlMtx[ m_leafLvlDim.m_nodeLvl ] );
			
			#ifdef DEBUG
// 				insertedPlaceholders += m_placeholders.size();
			#endif
			
			// Insert all leaf level pending nodes and placeholders.
			m_front.splice( m_front.end(), m_placeholders );
		}
		
		if( !m_front.empty() )
		{
			// The level from which the placeholders will be substituted is the one with max size, in order to maximize
			// placeholder substitution.
			size_t maxSize = 1;
			int substitutionLvl = 0;
			for( int i = 1; i < m_perLvlInsertions.size(); ++i )
			{
				lock_guard< mutex > lock( m_perLvlMtx[ i ] );
				size_t lvlSize = m_perLvlInsertions[ i ].size();
				maxSize = std::max( maxSize, lvlSize );
				
				if( maxSize == lvlSize )
				{
					substitutionLvl = i;
				}
			}
			
			#ifdef DEBUG
// 				long expectedToSubstitute;
// 				{
// 					lock_guard< mutex > lock( m_perLvlMtx[ substitutionLvl ] );
// 					expectedToSubstitute = m_perLvlInsertions[ substitutionLvl ].size();
// 				}
				
// 				{
// 					stringstream ss; ss << "==== FRONT TRACKING START ====" << endl << "Front size: " << m_front.size()
// 										<< " Inserted placeholders: " << insertedPlaceholders << " Substitution lvl: "
// 										<< substitutionLvl << " Expected to substitute: " << expectedToSubstitute << endl
// 										<< endl;
// 					HierarchyCreationLog::logDebugMsg( ss.str() );
// 				}
			#endif
			
			bool transactionNeeded = AllocStatistics::totalAllocated() > m_memoryLimit;
			m_releaseFlag = transactionNeeded;
			
			if( transactionNeeded )
			{
				m_sql.beginTransaction();
				
				#ifdef DEBUG
// 					stringstream ss; ss << "Front size: " << m_front.size() << endl << endl;
// 					HierarchyCreationLog::logDebugMsg( ss.str() );
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
// 					HierarchyCreationLog::logDebugMsg( "track starts\n" );
// 					assertFrontIterator( frontIt );
// 					assertNode( *frontIt->m_octreeNode, frontIt->m_morton );
				}
				#endif
				
				trackNode( frontIt, lastParent, substitutionLvl, renderer, projThresh );
				
				#ifdef DEBUG
				{
// 					HierarchyCreationLog::logDebugMsg( "track ends\n" );
				}
				#endif
			}
			
			if( transactionNeeded )
			{
				m_sql.endTransaction();
			}
			
			#ifdef DEBUG
// 			{
// 				stringstream ss; ss << "==== FRONT TRACKING END ====" << endl << "Front size: " << m_front.size()
// 									<< " Substitution lvl: " << substitutionLvl << " Placeholders after: "
// 									<< m_nPlaceholders << " Expected to substitute: " << expectedToSubstitute
// 									<< " Substituted: " << m_nSubstituted << " Persisted: " << m_persisted << endl
// 									<< endl;
// 				HierarchyCreationLog::logDebugMsg( ss.str() );
// // 				assert( m_nSubstituted >= expectedToSubstitute && "Expected more placeholders to be substituted" );
// 			}
			#endif
		}
		
		if( m_releaseFlag && m_persisted == 0 )
		{
			#ifdef DEBUG
// 			{
// 				stringstream ss; ss << "No more nodes can be persisted." << endl << endl;
// 				HierarchyCreationLog::logDebugMsg( ss.str() );
// 			}
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
				#ifdef DEBUG
// 					++m_nPlaceholders;
				#endif
				
				frontIt++;
				return;
			}
			#ifdef DEBUG
// 			else
// 			{
// 				assertFrontIterator( frontIt );
// 			}
			#endif
		}
		
		#ifdef DEBUG
// 		if( frontNode.m_octreeNode == nullptr )
// 		{
// 			cout << frontNode.m_morton.toString() << endl << endl;
// 			cout.flush();
// // 			stringstream ss; ss << "Null node: " << frontNode.m_morton.getPathToRoot( true ) << endl;
// // 			HierarchyCreationLog::logAndFail( ss.str() );
// 		}
		#endif
		
		Node& node = *frontNode.m_octreeNode;
		Morton& morton = frontNode.m_morton;
		OctreeDim nodeLvlDim( m_leafLvlDim, morton.getLevel() );
		
		Node* parentNode = node.parent();
		
		#ifdef DEBUG
		{
			
// 			stringstream ss; ss << "Tracking: " << morton.toString() << endl << endl;
// 			HierarchyCreationLog::logDebugMsg( ss.str() );
		}
		#endif
		
		// If parentNode == lastParent, prunning was not sucessful for a sibling of the current node, so the prunning
		// check can be skipped.
		if( parentNode != nullptr && parentNode != lastParent )  
		{
			OctreeDim parentLvlDim( nodeLvlDim, nodeLvlDim.m_nodeLvl - 1 );
			Morton parentMorton = *morton.traverseUp();
			
			#ifdef DEBUG
			{
// 				assertNode( node, morton );
			}
			#endif
			
			if( checkPrune( parentMorton, parentNode, parentLvlDim, frontIt, substitutionLvl, renderer, projThresh ) )
			{
				#ifdef DEBUG
// 				{
// 					stringstream ss; ss << "Prunning: " << morton.toString() << " Parent: " << parentMorton.toString()
// 										<< endl << endl;
// 					HierarchyCreationLog::logDebugMsg( ss.str() );
// 				}
				#endif
				
				prune( frontIt, nodeLvlDim, parentNode, renderer );
				lastParent = parentNode;
				
				#ifdef DEBUG
				{
// 					auto tempIt = prev( frontIt );
// 					assertNode( *tempIt->m_octreeNode, tempIt->m_morton );
// 					HierarchyCreationLog::logDebugMsg( "a prune\n" );
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
// 					assertNode( *substituteCandidate.m_octreeNode, substituteCandidate.m_morton );
				}
				#endif
				
				node = substituteCandidate;
				
				#ifdef DEBUG
				{
// 					assertNode( *node.m_octreeNode, node.m_morton );
				}
				#endif
				
				substitutionLvlList.erase( substitutionLvlList.begin() );
				
				#ifdef DEBUG
				{
// 					assertNode( *node.m_octreeNode, node.m_morton );
				}
				#endif
				
				#ifdef DEBUG
// 					++m_nSubstituted;
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
				#ifdef DEBUG
					bool wasPlaceholder = siblingIter->m_octreeNode == &m_placeholder;
					bool wasSubstituted = false;
				#endif
				
				if( siblingIter->m_octreeNode == &m_placeholder )
				{
					#ifdef DEBUG
						wasSubstituted =
					#endif
					substitutePlaceholder( *siblingIter, substitutionLvl );
				}
				
				#ifdef DEBUG
					if( siblingIter->m_octreeNode == nullptr )
					{
						stringstream ss; ss << siblingIter->m_morton.getPathToRoot( true ) << "Iter with null node pointer."
							<< "Was placeholder: " << wasPlaceholder << " Was substituted: " << wasSubstituted << endl << endl;
						HierarchyCreationLog::logAndFail( ss.str() );
					}
					//
				#endif
					
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
// 			int toRelease = parentNode->child().size();
// 			ulong processedBefore = m_processedNodes;
// 			bool released = false;
			
// 			assertNode( *frontIt->m_octreeNode, frontIt->m_morton );
			
// 			stringstream ss; ss << "Prunning group of " << frontIt->m_morton.toString() << endl << endl;
// 			HierarchyCreationLog::logDebugMsg( ss.str() );
		#endif
		
		while( frontIt != m_front.end() && frontIt->m_octreeNode->parent() == parentNode )
		{
			++m_processedNodes;
			
			#ifdef DEBUG
// 			{
// 				stringstream ss; ss << "Releasing " << frontIt->m_morton.getPathToRoot( true ) << endl;
// 				HierarchyCreationLog::logDebugMsg( ss.str() );
// 			}
			#endif
			
			frontIt = m_front.erase( frontIt );
		}
		
		if( m_releaseFlag )
		{
			#ifdef DEBUG
// 				stringstream ss; ss << "Should release" << endl << endl;
// 				HierarchyCreationLog::logDebugMsg( ss.str() );
			#endif
			
			if( AllocStatistics::totalAllocated() > m_memoryLimit )
			{
				#ifdef DEBUG
// 					released = true;
				#endif
				
				releaseSiblings( parentNode->child(), nodeLvlDim );
			}
			else
			{
				m_releaseFlag = false;
			}
		}
		
		#ifdef DEBUG
// 		{
// 			if( released && toRelease != m_processedNodes - processedBefore )
// 			{
// 				stringstream ss; ss << m_front.front().m_morton.getPathToRoot( true ) << " expected parent: "
// 									<< parentNode << " found: " << m_front.front().m_octreeNode->parent() << endl
// 									<< "All persisted nodes should also be released." << endl << endl;
// 				HierarchyCreationLog::logAndFail( ss.str() );
// 			}
// 		}
		#endif
		
		FrontNode frontNode( *parentNode, parentMorton );
		
		#ifdef DEBUG
		{
// 			assertNode( *frontNode.m_octreeNode, frontNode.m_morton );
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
// 				stringstream ss; ss << "2DB: " << siblingMorton.getPathToRoot( true ) << endl;
// 				HierarchyCreationLog::logDebugMsg( ss.str() );
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