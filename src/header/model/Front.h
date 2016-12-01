#ifndef FRONT_H
#define FRONT_H

#include <list>
#include "O1OctreeNode.h"
#include "OctreeDimensions.h"
#include "OctreeStats.h"
// #include "renderers/StreamingRenderer.h"
#include "splat_renderer/splat_renderer.hpp"
#include "NodeLoader.h"
#include "Profiler.h"
#include "StackTrace.h"
#include "phongshader.hpp"
#include "HierarchyCreationLog.h"

// Definitions to turn on debug logging for each Front operation.
// #define INSERTION_DEBUG
// #define SUBSTITUTION_DEBUG
// #define ORDERING_DEBUG
// #define RENDERING_DEBUG
// #define FRONT_TRACKING_DEBUG
// #define PRUNING_DEBUG

// Turn on asynchronous GPU node loading.
#define ASYNC_LOAD

// Number of threads in front tracking.
#define N_THREADS 1

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
	template< typename Morton >
	class Front
	{
	public:
		using Node = O1OctreeNode< Surfel >;
		using NodeArray = Array< Node >;
		using OctreeDim = OctreeDimensions< Morton >;
		using Renderer = SplatRenderer;
		using NodeLoader = model::NodeLoader< Point >;
		
		/** The node type that is used in front. */
		typedef struct FrontNode
		{
			FrontNode( Node& node, const Morton& morton )
			: m_octreeNode( &node ),
			m_morton( morton )
			{}
			
			~FrontNode()
			{}
			
			FrontNode& operator=( const FrontNode& other )
			{
				m_octreeNode = other.m_octreeNode;
				m_morton = other.m_morton;
				
				return *this;
			}
			
			friend ostream& operator<<( ostream& out, const FrontNode& node )
			{
				out << "Morton:" << node.m_morton.getPathToRoot( true ) << "Node: " << endl << *node.m_octreeNode
					<< endl << endl;
				return out;
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
		Front( const string& dbFilename, const OctreeDim& leafLvlDim, const int nHierarchyCreationThreads,
			   NodeLoader& loader, const ulong memoryLimit );
		
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
		/** Front segment. Each segment contains:
		 * a node list which have its node data,
		 * a morton code in the deepest hierarchy level that indicates its morton code boundary,
		 * a flag that indicates if it is substituting placeholders,
		 * a flag that indicates if it is rendering points, i.e., updating the GPU point buffer segment. */
		typedef struct Segment
		{
			Segment()
			: m_isSubstituting( false )
			{}
			
			FrontList m_front;
			uint m_index;
			bool m_isSubstituting;
		} Segment;
		
		void trackNode( FrontListIter& frontIt, Segment& segment, Node*& lastParent, int substitutionLvl,
						Renderer& renderer, const Float projThresh );
		
		/** Substitute a placeholder with the first node of the given substitution level. */
		bool substitutePlaceholder( FrontNode& node, int substitutionLvl );
		
		bool checkPrune( const Morton& parentMorton, Node* parentNode, const OctreeDim& parentLvlDim,
						 FrontListIter& frontIt, Segment& segment, int substituionLvl, Renderer& renderer,
					const Float projThresh );
		
		void prune( FrontListIter& frontIt, Segment& segment, Node* parentNode, Renderer& renderer );
		
		bool checkBranch( const OctreeDim& nodeLvlDim, Node& node, const Morton& morton, Renderer& renderer,
						  const Float projThresh, bool& out_isCullable );
		
		void branch( FrontListIter& iter, Segment& segment, Node& node, const OctreeDim& nodeLvlDim, Renderer& renderer );
		
		void setupNodeRendering( FrontListIter& iter, const FrontNode& frontNode, Segment& segment, Renderer& renderer );
		
		void setupNodeRenderingNoFront( FrontListIter& iter, Node& node, Renderer& renderer ) const;
		
		bool isSubstIdxInRange( const uint rangeStartIdx, const uint rangeSize ) const
		{
			return m_substitutionSegIdx >= rangeStartIdx && m_substitutionSegIdx < rangeStartIdx + rangeSize;
		}
		
		#ifdef ORDERING_DEBUG
			void assertFrontIterator( const FrontListIter& iter, const FrontList& front )
			{
				if( iter != front.begin() )
				{
					Morton& currMorton = iter->m_morton;
					Morton& prevMorton = prev( iter )->m_morton;
					
					uint currLvl = currMorton.getLevel();
					uint prevLvl = prevMorton.getLevel();
					
					bool isPlaceholder = ( iter->m_octreeNode == &m_placeholder );
					stringstream ss;
					if( currLvl < prevLvl )
					{
						Morton prevAncestorMorton = prevMorton.getAncestorInLvl( currLvl );
						if( currMorton <= prevAncestorMorton )
						{
							ss  << "Front order compromised. Prev: " << prevAncestorMorton.getPathToRoot( true )
								<< "Curr: " << currMorton.getPathToRoot( true ) << " is placeholder? "
								<< isPlaceholder << endl;
							HierarchyCreationLog::logAndFail( ss.str() );
						}
					}
					else if( currLvl > prevLvl )
					{
						Morton currAncestorMorton = currMorton.getAncestorInLvl( prevLvl );
						if( currAncestorMorton <= prevMorton  )
						{
							ss  << "Front order compromised. Prev: " << prevMorton.getPathToRoot( true )
								<< "Curr: " << currAncestorMorton.getPathToRoot( true ) << " is placeholder? "
								<< isPlaceholder << endl;
							HierarchyCreationLog::logAndFail( ss.str() );
						}
					}
					else
					{
						if( currMorton <= prevMorton )
						{
							ss  << "Front order compromised. Prev: " << prevMorton.getPathToRoot( true )
								<< "Curr: " << currMorton.getPathToRoot( true ) << " is placeholder? "
								<< isPlaceholder << endl;
							HierarchyCreationLog::logAndFail( ss.str() );
						}
					}
				}
// 				assertNode( *iter->m_octreeNode, iter->m_morton );
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
		
		/** Node used as a placeholder in the front. It is used whenever it is known that a node should occupy a given
		 * position, but the node itself is not defined yet because the hierarchy creation algorithm have not reached
		 * the needed level. */
		Node m_placeholder;
		
		/** Mutexes to synchronize m_perLvlInsertions operations. */
		vector< mutex > m_perLvlMtx;
		
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
		
		/** Front segment array. */
		Array< Segment > m_segments;
		
		/** Contains segment indices used to map threads in segments. m_threadSegmentMap[ t ] contains the segment mapped
		 * to thread t. */
		Array< uint > m_threadSegmentMap;
		
		NodeLoader& m_nodeLoader;
		
		/** Dimensions of the octree nodes at deepest level. */
		OctreeDim m_leafLvlDim;
		
		/** Memory usage limit. Used to turn on/off node release. */
		ulong m_memoryLimit;
		
		/** Maximum nodes per front segment. */
		uint m_nNodesPerSegment;
		
		/** Segment where the placeholder substitution will occur. */
		uint m_substitutionSegIdx;
		
		/** Segment idx where new placeholders will be appended. */
		uint m_appendSegIdx;
		
		/** Index of the front segment selection. */
		uint m_segSelectionIdx;
		
		/** Size of the front segment selection. */
		uint m_segSelectionSize;
		
		/** Number of threads used for front tracking. */
		uint m_nFrontThreads;
		
		/** Indicates that all leaf level nodes are already loaded. */
		atomic_bool m_leafLvlLoadedFlag;
	};
	
	template< typename Morton >
	inline Front< Morton >::Front( const string& dbFilename, const OctreeDim& leafLvlDim,
								   const int nHierarchyCreationThreads, NodeLoader& loader, const ulong memoryLimit )
	: m_leafLvlDim( leafLvlDim ),
	m_memoryLimit( memoryLimit ),
	m_currentIterInsertions( nHierarchyCreationThreads ),
	m_currentIterPlaceholders( nHierarchyCreationThreads ),
	m_perLvlInsertions( leafLvlDim.m_nodeLvl + 1 ),
	m_perLvlMtx( leafLvlDim.m_nodeLvl + 1 ),
	m_nNodesPerSegment( 9999999 ),
	m_segments( 1 ),
	m_substitutionSegIdx( 0 ),
	m_appendSegIdx( 0 ),
	m_segSelectionIdx( 0 ),
	m_segSelectionSize( 1 ),
	m_nFrontThreads( N_THREADS ),
	m_threadSegmentMap( N_THREADS, uint( 0 ) ),
	m_leafLvlLoadedFlag( false ),
	m_nodeLoader( loader )
	{
		uint i = 0;
		for( Segment& segment : m_segments )
		{
			segment.m_index = i++;
		}
		
		m_segments[ 0 ].m_isSubstituting = true;
	}

	template< typename Morton >
	void Front< Morton >::notifyLeafLvlLoaded()
	{
		m_leafLvlLoadedFlag = true;
	}
	
	template< typename Morton >
	inline bool Front< Morton >::isReleasing()
	{
		return m_nodeLoader.isReleasing();
	}
	
	template< typename Morton >
	inline void Front< Morton >::insertIntoBufferEnd( Node& node, const Morton& morton, int threadIdx )
	{
		#ifdef INSERTION_DEBUG
		{
			stringstream ss; ss << "Inserting " << morton.getPathToRoot( true ) << endl << endl;
			HierarchyCreationLog::logDebugMsg( ss.str() );
		}
		#endif
		
		FrontList& list = m_currentIterInsertions[ threadIdx ];
		FrontNode frontNode( node, morton );
		
		list.push_back( frontNode );
	}
	
	template< typename Morton >
	inline typename Front< Morton >::FrontListIter Front< Morton >::getIteratorToBufferBegin( int threadIdx )
	{
		return m_currentIterInsertions[ threadIdx ].begin();
	}
	
	template< typename Morton >
	inline void Front< Morton >::insertIntoBuffer( FrontListIter& iter, Node& node, const Morton& morton,
													int threadIdx )
	{
		#ifdef INSERTION_DEBUG
		{
			stringstream ss; ss << "Inserting " << morton.getPathToRoot( true ) << endl << endl;
			HierarchyCreationLog::logDebugMsg( ss.str() );
		}
		#endif
		
		FrontList& list = m_currentIterInsertions[ threadIdx ];
		FrontNode frontNode( node, morton );
		
		list.insert( iter, frontNode );
	}
	
	template< typename Morton >
	inline void Front< Morton >::insertPlaceholder( const Morton& morton, int threadIdx )
	{
		assert( morton.getLevel() == m_leafLvlDim.m_nodeLvl && "Placeholders should be in hierarchy's deepest level." );
		
		FrontList& list = m_currentIterPlaceholders[ threadIdx ];
		list.push_back( FrontNode( m_placeholder, morton ) );
	}
	
	template< typename Morton >
	inline void Front< Morton >::notifyInsertionEnd( uint dispatchedThreads )
	{
		if( dispatchedThreads > 0 )
		{
			int lvl = -1;
			for( FrontList& list : m_currentIterInsertions )
			{
				if( !list.empty() )
				{
					lvl = list.front().m_morton.getLevel();
					break;
				}
			}
			
			if( lvl != -1 )
			{
				lock_guard< mutex > lock( m_perLvlMtx[ lvl ] );
			
				for( FrontList& list : m_currentIterInsertions )
				{
					FrontList& lvlInsertions = m_perLvlInsertions[ lvl ];
					// Move nodes to the per-level sorted buffer.
					lvlInsertions.splice( lvlInsertions.end(), list );
				}
			}
			
			{
				lock_guard< mutex > lock( m_perLvlMtx[ m_leafLvlDim.m_nodeLvl ] );
				
				// Move placeholders to the sorted buffer.
				for( FrontList& list : m_currentIterPlaceholders )
				{
					m_placeholders.splice( m_placeholders.end(), list );
				}
			}
		}
	}
	
	template< typename Morton >
	inline FrontOctreeStats Front< Morton >::trackFront( Renderer& renderer, const Float projThresh )
	{
		auto start = Profiler::now();
		
		renderer.begin_frame();
		
		Segment& appendSeg = m_segments[ m_appendSegIdx ];
		
		{
			lock_guard< mutex > lock( m_perLvlMtx[ m_leafLvlDim.m_nodeLvl ] );
			
			// Insert all leaf level placeholders.
			appendSeg.m_front.splice( appendSeg.m_front.end(), m_placeholders );
		}
		
		if( appendSeg.m_front.size() > m_nNodesPerSegment )
		{
			++m_appendSegIdx;
		}
		
		if( !m_segments[ 0 ].m_front.empty() )
		{
			// The level from which the placeholders will be substituted is the one with max size, in order to maximize
			// placeholder substitution.
			size_t maxSize = 1;
			int substitutionLvl = -1;
			for( int i = 0; i < m_perLvlInsertions.size(); ++i )
			{
				lock_guard< mutex > lock( m_perLvlMtx[ i ] );
				size_t lvlSize = m_perLvlInsertions[ i ].size();
				maxSize = std::max( maxSize, lvlSize );
				
				if( maxSize == lvlSize )
				{
					substitutionLvl = i;
				}
			}

			uint dispatchedThreads = isSubstIdxInRange( m_segSelectionIdx, m_segSelectionSize )
									 ? m_segSelectionSize : m_segSelectionSize + 1;
			Node* lastParent = nullptr; // Parent of last node. Used to optimize prunning check.
			
// 			#pragma omp parallel for num_threads(N_THREADS)
			for( int i = 0; i < dispatchedThreads; ++i )
			{
				uint segmentIdx = m_threadSegmentMap[ i ];
				
				Segment& segment = m_segments[ segmentIdx ];
				FrontList& front = segment.m_front;
				
				#ifdef FRONT_TRACKING_DEBUG
				{
					stringstream ss; ss << "Front size: " << front.size() << endl << endl;
					HierarchyCreationLog::logDebugMsg( ss.str() );
				}
				#endif
				
				for( FrontListIter iter = front.begin(); iter != front.end(); /**/ )
				{
					#ifdef FRONT_TRACKING_DEBUG
					{
						stringstream ss; ss << "Tracking " << iter->m_morton.getPathToRoot() << endl << endl;
						HierarchyCreationLog::logDebugMsg( ss.str() );
					}
					#endif
					
					#ifdef ORDERING_DEBUG
						assertFrontIterator( iter, front );
					#endif
					
					trackNode( iter, segment, lastParent, substitutionLvl, renderer, projThresh );
				}
			}
			
			m_nodeLoader.onIterationEnd();
		}
		
		renderer.render_frame();
		
		int traversalTime = Profiler::elapsedTime( start );
		
		start = Profiler::now();
		
		unsigned int numRenderedPoints = renderer.end_frame();
		
		// Select next frame's placeholder substitution segment.
		m_substitutionSegIdx = ( m_substitutionSegIdx + 1 ) % m_segments.size();
		
		if( m_segments[ m_substitutionSegIdx ].m_front.empty() )
		{
			m_substitutionSegIdx = 0;
		}
		
		// Select next frame's point update segment range.
		uint nextFrameFirstSeg = m_segSelectionIdx + m_segSelectionSize;
		
		uint segSelectionStart = ( nextFrameFirstSeg == m_segments.size() ||
								   m_segments[ nextFrameFirstSeg ].m_front.empty() )
								 ? 0 : nextFrameFirstSeg;
								 
		bool substIdxInRangeFlag = isSubstIdxInRange( segSelectionStart, m_nFrontThreads );
		uint expectedSelectionSize = ( substIdxInRangeFlag ) ? m_nFrontThreads : m_nFrontThreads - 1;
		
		uint segSelectionSize = ( segSelectionStart + expectedSelectionSize <= m_appendSegIdx + 1 )
								? expectedSelectionSize : m_appendSegIdx + 1 - segSelectionStart;

		m_segSelectionIdx = segSelectionStart;
		m_segSelectionSize = segSelectionSize;
		
		for( int i = 0; i < segSelectionSize; ++i )
		{
			m_threadSegmentMap[ i ] = segSelectionStart + i;
			Segment& segment = m_segments[ segSelectionStart + i ];
			segment.m_isSubstituting = false;
		}
		
		Segment& substSegment = m_segments[ m_substitutionSegIdx ];
		substSegment.m_isSubstituting = true;
		
		if( !substIdxInRangeFlag )
		{
			m_threadSegmentMap[ segSelectionSize ] = m_substitutionSegIdx;
		}
		
		uint nNodes = 0;
		// Not optimized. Probably it doesn't need to, anyway.
		for( Segment& segment : m_segments )
		{
			nNodes += segment.m_front.size();
		}
		
		int renderingTime = Profiler::elapsedTime( start );
		
		#ifdef FRONT_TRACKING_DEBUG
		{
			HierarchyCreationLog::logDebugMsg( "===== FRONT TRACKING END =====\n\n" );
		}
		#endif
		
		return FrontOctreeStats( traversalTime, renderingTime, numRenderedPoints, nNodes );
	}
	
	template< typename Morton >
	inline void Front< Morton >
	::trackNode( FrontListIter& frontIt, Segment& segment, Node*& lastParent, int substitutionLvl, Renderer& renderer,
				 const Float projThresh )
	{
		FrontNode& frontNode = *frontIt;
		
		if( segment.m_isSubstituting && frontNode.m_octreeNode == &m_placeholder )
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
		
		Node* parentNode = node.parent();
		
		// If parentNode == lastParent, prunning was not sucessful for a sibling of the current node, so the prunning
		// check can be skipped.
		if( parentNode != nullptr && parentNode != lastParent )  
		{
			OctreeDim parentLvlDim( nodeLvlDim, nodeLvlDim.m_nodeLvl - 1 );
			Morton parentMorton = *morton.traverseUp();
			
			if( checkPrune( parentMorton, parentNode, parentLvlDim, frontIt, segment, substitutionLvl, renderer,
							projThresh ) )
			{
				prune( frontIt, segment, parentNode, renderer );
				lastParent = parentNode;
				
				return;
			}
			lastParent = parentNode;
		}
		
		bool isCullable = false;
		
		if( checkBranch( nodeLvlDim, node, morton, renderer, projThresh, isCullable ) )
		{
			branch( frontIt, segment, node, nodeLvlDim, renderer );
			return;
		}
		
		if( !isCullable )
		{
			#ifdef RENDERING_DEBUG
			{
				stringstream ss; ss << "Rendering: " << endl << morton.getPathToRoot( true ) << endl << node << endl
					<< endl;
				HierarchyCreationLog::logDebugMsg( ss.str() );
			}
			#endif
			
			// No prunning or branching done. Just send the current front node for rendering.
			setupNodeRenderingNoFront( frontIt, node, renderer );
			return;
		}
		
		frontIt++;
	}
	
	template< typename Morton >
	inline bool Front< Morton >::substitutePlaceholder( FrontNode& node, int substitutionLvl )
	{
		assert( node.m_octreeNode == &m_placeholder && "Substitution paramenter should be a placeholder node" );
		
		if( substitutionLvl != -1 )
		{
			lock_guard< mutex > lock( m_perLvlMtx[ substitutionLvl ] );
			
			FrontList& substitutionLvlList = m_perLvlInsertions[ substitutionLvl ];
			
			if( !substitutionLvlList.empty() )
			{
				FrontNode& substituteCandidate = substitutionLvlList.front();
				
				if( node.m_morton.isDescendantOf( substituteCandidate.m_morton ) )
				{
					#ifdef SUBSTITUTION_DEBUG
						stringstream ss; ss << "Substituting placeholder " << node.m_morton.getPathToRoot( true )
							<< " by " << substituteCandidate.m_morton.getPathToRoot( true ) << endl << endl;
						HierarchyCreationLog::logDebugMsg( ss.str() );
					#endif
					
					node = substituteCandidate;
					
					#ifdef ASYNC_LOAD
						m_nodeLoader.asyncLoad( *node.m_octreeNode, omp_get_thread_num() );
					#else
						node.m_octreeNode->loadGPU();
					#endif
						
					substitutionLvlList.erase( substitutionLvlList.begin() );
					
					return true;
				}
				else 
				{
					return false;
				}
			}
		}
		
		return false;
	}
	
	template< typename Morton >
	inline bool Front< Morton >
	::checkPrune( const Morton& parentMorton, Node* parentNode, const OctreeDim& parentLvlDim,
				  FrontListIter& frontIt, Segment& segment, int substitutionLvl, Renderer& renderer,
			   const Float projThresh )
	{
		AlignedBox3f parentBox = parentLvlDim.getMortonBoundaries( parentMorton );
		
		bool pruneFlag = false;
		if( renderer.isCullable( parentBox ) )
		{
			pruneFlag = true;
			
			#ifdef PRUNING_DEBUG
			{
				stringstream ss; ss << parentMorton.getPathToRoot() << ": CULLABLE." << endl << endl;
				HierarchyCreationLog::logDebugMsg( ss.str() );
			}
			#endif
		}
		else
		{
			#ifdef PRUNING_DEBUG
			{
				stringstream ss; ss << parentMorton.getPathToRoot() << ": NOT CULLABLE." << endl << endl;
				HierarchyCreationLog::logDebugMsg( ss.str() );
			}
			#endif
			
			if( renderer.isRenderable( parentBox, projThresh ) )
			{
				#ifdef PRUNING_DEBUG
				{
					stringstream ss; ss << "Parent " << parentMorton.getPathToRoot() << ": RENDERABLE." << endl
						<< endl;
					HierarchyCreationLog::logDebugMsg( ss.str() );
				}
				#endif
				
				pruneFlag = true;
			}
		}
		
		if( pruneFlag )
		{
			int nSiblings = 0;
			FrontListIter siblingIter = frontIt;
			while( siblingIter != segment.m_front.end() )
			{
				if( segment.m_isSubstituting && siblingIter->m_octreeNode == &m_placeholder )
				{
					substitutePlaceholder( *siblingIter, substitutionLvl );
				}
				
				if( siblingIter++->m_octreeNode->parent() != parentNode )
				{
					break;
				}
				
				++nSiblings;
			}
			
			// The last sibling group cannot be prunned when the leaf level is not loaded yet. It can be incomplete
			// at that time and all the sibling nodes should be in front before prunning.
			if( ( !m_leafLvlLoadedFlag && siblingIter == segment.m_front.end() ) ||
				nSiblings != parentNode->child().size() )
			{
				#ifdef PRUNING_DEBUG
				{
					stringstream ss; ss << parentMorton.getPathToRoot() << ": last sibling group."
						<< endl << endl;
					HierarchyCreationLog::logDebugMsg( ss.str() );
				}
				#endif
				
				pruneFlag = false;
			}
		}
		
		if( pruneFlag && parentNode->loadState() != Node::LOADED )
		{
			#ifdef ASYNC_LOAD
				m_nodeLoader.asyncLoad( *parentNode, omp_get_thread_num() );
			#else
				parentNode->loadGPU();
			#endif
			
			#ifdef PRUNING_DEBUG
			{
				stringstream ss; ss << parentMorton.getPathToRoot() << ": not loaded."
					<< endl << endl;
				HierarchyCreationLog::logDebugMsg( ss.str() );
			}
			#endif
				
			pruneFlag = false;
		}
		
		#ifdef PRUNING_DEBUG
		{
			stringstream ss; ss << parentMorton.getPathToRoot() << ": prunning successful? " << pruneFlag
				<< endl << endl;
			HierarchyCreationLog::logDebugMsg( ss.str() );
		}
		#endif
		
		return pruneFlag;
	}
	
	template< typename Morton >
	inline void Front< Morton >::prune( FrontListIter& frontIt, Segment& segment, Node* parentNode,
											   Renderer& renderer )
	{
		Morton parentMorton = *frontIt->m_morton.traverseUp();
		
		while( frontIt != segment.m_front.end() && frontIt->m_octreeNode->parent() == parentNode )
		{
			frontIt = segment.m_front.erase( frontIt );
		}
		
		if( AllocStatistics::totalAllocated() > m_memoryLimit )
		{
			#ifdef ASYNC_LOAD
				m_nodeLoader.asyncRelease( std::move( parentNode->child() ), omp_get_thread_num() );
			#else
				parentNode->releaseChildren();
			#endif
		}
		else
		{
			if( m_nodeLoader.reachedGpuMemQuota() )
			{
				for( Node& node : parentNode->child() )
				{
					#ifdef ASYNC_LOAD
						m_nodeLoader.asyncUnload( node, omp_get_thread_num() );
					#else
						node.unloadGPU();
					#endif
				}
			}
		}
		
		FrontNode frontNode( *parentNode, parentMorton );
		setupNodeRendering( frontIt, frontNode, segment, renderer );
	}
	
	template< typename Morton >
	inline bool Front< Morton >
	::checkBranch( const OctreeDim& nodeLvlDim, Node& node, const Morton& morton, Renderer& renderer,
				   const Float projThresh, bool& out_isCullable )
	{
		AlignedBox3f box = nodeLvlDim.getMortonBoundaries( morton );
		out_isCullable = renderer.isCullable( box );
		
		if( !node.isLeaf() && !node.child().empty() )
		{
			NodeArray& children = node.child();
			if( children[ 0 ].loadState() == Node::LOADED )
			{
				return !renderer.isRenderable( box, projThresh ) && !out_isCullable;
			}
			else
			{
				for( Node& child : children )
				{
					#ifdef ASYNC_LOAD
						m_nodeLoader.asyncLoad( child, omp_get_thread_num() );
					#else
						child.loadGPU();
					#endif
				}
			}
		}
		
		return false;
	}
	
	template< typename Morton >
	inline void Front< Morton >::branch( FrontListIter& frontIt,  Segment& segment, Node& node,
										 const OctreeDim& nodeLvlDim, Renderer& renderer )
	{
		frontIt = segment.m_front.erase( frontIt );
		
		OctreeDim childLvlDim( nodeLvlDim, nodeLvlDim.m_nodeLvl + 1 );
		NodeArray& children = node.child();
		
		for( int i = 0; i < children.size(); ++i )
		{
			Node& child = children[ i ];
			AlignedBox3f box = childLvlDim.getNodeBoundaries( child );
			FrontNode frontNode( child, childLvlDim.calcMorton( child ) );
			
			assert( frontNode.m_morton.getBits() != 1 && "Inserting root node into front (branch)." );
			
			if( !renderer.isCullable( box ) )
			{
				setupNodeRendering( frontIt, frontNode, segment, renderer );
			}
			else
			{
				segment.m_front.insert( frontIt, frontNode );
			}
		}
	}
	
	template< typename Morton >
	inline void Front< Morton >::setupNodeRendering( FrontListIter& frontIt, const FrontNode& frontNode,
													 Segment& segment, Renderer& renderer )
	{
		segment.m_front.insert( frontIt, frontNode );
		
		Node* node = frontNode.m_octreeNode;
		
		#ifdef RENDERING_DEBUG
		{
			stringstream ss; ss << "Rendering: " << endl << frontNode.m_morton.getPathToRoot( true ) << endl <<
				*node << endl << endl;
			HierarchyCreationLog::logDebugMsg( ss.str() );
		}
		#endif
		
		if( node->loadState() == Node::LOADED )
		{
			#ifdef TUCANO_RENDERER
				renderer.render_cloud_tucano( node->getContents() );
			#else
				renderer.render_cloud( node->cloud() );
			#endif
		}
	}
	
	template< typename Morton >
	inline void Front< Morton >::setupNodeRenderingNoFront( FrontListIter& frontIt, Node& node,
															Renderer& renderer ) const
	{
		frontIt++;
		
		if( node.loadState() == Node::LOADED )
		{
			#ifdef TUCANO_RENDERER
				renderer.render_cloud_tucano( node.getContents() );
			#else
				renderer.render_cloud( node.cloud() );
			#endif
		}
	}
}

#undef INSERTION_DEBUG
#undef SUBSTITUTION_DEBUG
#undef ORDERING_DEBUG
#undef RENDERING_DEBUG
#undef FRONT_TRACKING_DEBUG
#undef PRUNING_DEBUG

#undef ASYNC_LOAD

#endif