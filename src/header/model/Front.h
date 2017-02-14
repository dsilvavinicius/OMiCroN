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
#include "TextEffect.h"
#include "global_malloc.h"

// Definitions to turn on debug logging for each Front operation.
// #define INSERTION_DEBUG
// #define SUBSTITUTION_DEBUG
// #define ORDERING_DEBUG
// #define RENDERING_DEBUG
// #define FRONT_TRACKING_DEBUG
// #define PRUNING_DEBUG
// #define BRANCHING_DEBUG

// Number of front nodes processed per frame.
#define FRONT_NODES_PER_FRAME 100

// Turn on asynchronous GPU node loading.
#define ASYNC_LOAD

// Cut rendering methods.
#define RENDER_ENTIRE_CUT 0 // Renders the entire cut. The default and correct method to visualize the point cloud.
#define RENDER_OLD_CUT_ONLY 1 // Debug. Renders only nodes that were in cuts rendered already.
#define RENDER_NEW_CUT_ONLY 2 // Debug. Renders only nodes at the end of the front that were added to the cut recently
							  // and were not rendered yet.

// Choosen cut rendering method
#define CUT_RENDERING_METHOD RENDER_ENTIRE_CUT

// Render an id text in each rendered node.
// #define NODE_ID_TEXT

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
		void trackNode( FrontListIter& frontIt, Node*& lastParent, int substitutionLvl, Renderer& renderer,
						const Float projThresh );
		
		/** Substitute a placeholder with the first node of the given substitution level. */
		bool substitutePlaceholder( FrontNode& node, int substitutionLvl );
		
		bool checkPrune( const Morton& parentMorton, Node* parentNode, const OctreeDim& parentLvlDim,
						 FrontListIter& frontIt, int substituionLvl, Renderer& renderer,
					const Float projThresh, bool& out_isCullable );
		
		void prune( FrontListIter& frontIt, Node* parentNode, const bool parentIsCullable, Renderer& renderer );
		
		bool checkBranch( const OctreeDim& nodeLvlDim, Node& node, const Morton& morton, Renderer& renderer,
						  const Float projThresh, bool& out_isCullable );
		
		void branch( FrontListIter& iter, Node& node, const OctreeDim& nodeLvlDim, Renderer& renderer );
		
		void setupNodeRendering( FrontListIter& iter, const FrontNode& frontNode, Renderer& renderer );
		
		void setupNodeRenderingNoFront( const Morton& moton, Node& node, Renderer& renderer );
		
		void unloadInGpu( Node& node );
		
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
		
		/** The internal front datastructure. Contains all FrontNodes. */
		FrontList m_front;
		
		/** Iterator used to resume front processing from previous frame. */
		FrontListIter m_frontIter;
		
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
		
		NodeLoader& m_nodeLoader;
		
		/** Dimensions of the octree nodes at deepest level. */
		OctreeDim m_leafLvlDim;
		
		/** Memory usage limit. Used to turn on/off node release. */
		ulong m_memoryLimit;
		
		/** Indicates that all leaf level nodes are already loaded. */
		atomic_bool m_leafLvlLoadedFlag;
		
		#ifdef NODE_ID_TEXT
			TextEffect m_textEffect;
			vector< pair< string, Vector4f >, TbbAllocator< pair< string, Vector4f > > > m_nodeIds;
		#endif
			
		#if CUT_RENDERING_METHOD == RENDER_NEW_CUT_ONLY || CUT_RENDERING_METHOD == RENDER_OLD_CUT_ONLY
			Morton m_lastRendered;
		#endif
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
	m_leafLvlLoadedFlag( false ),
	m_nodeLoader( loader )
	{
		m_frontIter = m_front.end();
		
		#ifdef NODE_ID_TEXT
		{
			m_textEffect.initialize( "shaders/Inconsolata.otf" );
		}
		#endif
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
		
		{
			lock_guard< mutex > lock( m_perLvlMtx[ m_leafLvlDim.m_nodeLvl ] );
			
			// Insert all leaf level placeholders.
			m_front.splice( m_front.end(), m_placeholders );
		}
		
		if( !m_front.empty() )
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

			Node* lastParent = nullptr; // Parent of last node. Used to optimize prunning check.
				
			#if defined FRONT_TRACKING_DEBUG || defined RENDERING_DEBUG
			{
				stringstream ss; ss << "===== FRONT TRACKING BEGINS =====" << endl << "Size: " << m_front.size()
					<< endl << endl;
				HierarchyCreationLog::logDebugMsg( ss.str() );
			}
			#endif
			
			if( m_frontIter == m_front.end() )
			{
				m_frontIter = m_front.begin();
				renderer.resetIterator();
			}
			
			for( int i = 0; m_frontIter != m_front.end() && i < FRONT_NODES_PER_FRAME; ++i )
			{
				#ifdef FRONT_TRACKING_DEBUG
				{
					stringstream ss; ss << "Tracking " << m_frontIter->m_morton.getPathToRoot() << endl
						<< *m_frontIter->m_octreeNode << endl << endl;
					HierarchyCreationLog::logDebugMsg( ss.str() );
				}
				#endif
				
				#ifdef ORDERING_DEBUG
					assertFrontIterator( m_frontIter, front );
				#endif
				
				trackNode( m_frontIter, lastParent, substitutionLvl, renderer, projThresh );
			}
			
			m_nodeLoader.onIterationEnd();
			renderer.render_frame();
		}
		
		#ifdef NODE_ID_TEXT
		{
			m_textEffect.setColor( Vector4f( 0.f, 0.f, 0.f, 1.f ) );
			
			glEnable( GL_BLEND );
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
			
			glCullFace( GL_BACK );
			glEnable( GL_CULL_FACE );
			
			glEnable( GL_DEPTH_TEST );
			
			Camera camera = renderer.camera();
			
			for( const pair< string, Vector4f >& nodeId : m_nodeIds )
			{
				m_textEffect.render( nodeId.first, nodeId.second, camera );
			}
			m_nodeIds.clear();
			
			glDisable( GL_BLEND );
			glDisable( GL_CULL_FACE );
			glDisable( GL_DEPTH_TEST );
		}
		#endif
		
		
		int traversalTime = Profiler::elapsedTime( start );
		
		start = Profiler::now();
		
		unsigned int numRenderedPoints = renderer.end_frame();
		
		int renderingTime = Profiler::elapsedTime( start );
		
		#ifdef FRONT_TRACKING_DEBUG
		{
			HierarchyCreationLog::logDebugMsg( "===== FRONT TRACKING END =====\n\n" );
		}
		#endif
		
		return FrontOctreeStats( traversalTime, renderingTime, numRenderedPoints, m_front.size() );
	}
	
	template< typename Morton >
	inline void Front< Morton >
	::trackNode( FrontListIter& frontIt, Node*& lastParent, int substitutionLvl, Renderer& renderer, const Float projThresh )
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
		
		Node* parentNode = node.parent();
		
		// If parentNode == lastParent, prunning was not sucessful for a sibling of the current node, so the prunning
		// check can be skipped.
		if( parentNode != nullptr && parentNode != lastParent )  
		{
			OctreeDim parentLvlDim( nodeLvlDim, nodeLvlDim.m_nodeLvl - 1 );
			Morton parentMorton = *morton.traverseUp();
			bool parentIsCullable;
			if( checkPrune( parentMorton, parentNode, parentLvlDim, frontIt, substitutionLvl, renderer,
							projThresh, parentIsCullable ) )
			{
				prune( frontIt, parentNode, parentIsCullable, renderer );
				lastParent = parentNode;
				
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
			setupNodeRenderingNoFront( morton, node, renderer );
		}
		else
		{
			#ifdef RENDERING_DEBUG
			{
				stringstream ss; ss << "Trying to remove from rendering (culled): " << morton.getPathToRoot() << endl << endl;
				HierarchyCreationLog::logDebugMsg( ss.str() );
			}
			#endif
			
			renderer.eraseFromList( node );
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
						node.m_octreeNode->loadInGpu();
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
				  FrontListIter& frontIt, int substitutionLvl, Renderer& renderer, const Float projThresh,
			   bool& out_isCullable )
	{
		#ifdef PRUNING_DEBUG
		{
			stringstream ss; ss << parentMorton.getPathToRoot() << " checking prune." << endl << parentNode << endl
				<< endl;
			HierarchyCreationLog::logDebugMsg( ss.str() );
		}
		#endif
		
		AlignedBox3f parentBox = parentLvlDim.getMortonBoundaries( parentMorton );
		
		bool pruneFlag = false;
		out_isCullable = renderer.isCullable( parentBox );
		if( out_isCullable )
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
// 			{
// 				stringstream ss; ss << parentMorton.getPathToRoot() << ": NOT CULLABLE." << endl << endl;
// 				HierarchyCreationLog::logDebugMsg( ss.str() );
// 			}
			#endif
			
			if( renderer.isRenderable( parentBox, projThresh ) )
			{
				#ifdef PRUNING_DEBUG
// 				{
// 					stringstream ss; ss << parentMorton.getPathToRoot() << ": RENDERABLE." << endl
// 						<< endl;
// 					HierarchyCreationLog::logDebugMsg( ss.str() );
// 				}
				#endif
				
				pruneFlag = true;
			}
			#ifdef PRUNING_DEBUG
// 			else
// 			{
// 				stringstream ss; ss << parentMorton.getPathToRoot() << ": NOT RENDERABLE." << endl << endl;
// 				HierarchyCreationLog::logDebugMsg( ss.str() );
// 			}
			#endif
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
			
			// The last sibling group cannot be prunned when the leaf level is not loaded yet. It can be incomplete
			// at that time and all the sibling nodes should be in front before prunning.
			if( ( !m_leafLvlLoadedFlag && siblingIter == m_front.end() ) ||
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
		
		if( pruneFlag && !parentNode->isLoaded() )
		{
			#ifdef ASYNC_LOAD
				parentNode->loadInGpu();
			#else
				parentNode->loadGPU();
			#endif
			
			#ifdef PRUNING_DEBUG
// 			{
// 				stringstream ss; ss << parentMorton.getPathToRoot() << ": not loaded."
// 					<< endl << endl;
// 				HierarchyCreationLog::logDebugMsg( ss.str() );
// 			}
			#endif
				
			pruneFlag = false;
		}
		
		#ifdef PRUNING_DEBUG
// 		{
// 			stringstream ss; ss << parentMorton.getPathToRoot() << ": prunning successful? " << pruneFlag
// 				<< endl << endl;
// 			HierarchyCreationLog::logDebugMsg( ss.str() );
// 		}
		#endif
		
		return pruneFlag;
	}
	
	template< typename Morton >
	inline void Front< Morton >::prune( FrontListIter& frontIt, Node* parentNode, const bool parentIsCullable,
										Renderer& renderer )
	{
		Morton parentMorton = *frontIt->m_morton.traverseUp();
		
		while( frontIt != m_front.end() && frontIt->m_octreeNode->parent() == parentNode )
		{
			#ifdef RENDERING_DEBUG
			{
				stringstream ss; ss << "Trying to remove from rendering (prunning): " << frontIt->m_morton.getPathToRoot() << endl << endl;
				HierarchyCreationLog::logDebugMsg( ss.str() );
			}
			#endif
			
			renderer.eraseFromList( *frontIt->m_octreeNode );
			frontIt = m_front.erase( frontIt );
		}
		
// 		if( AllocStatistics::totalAllocated() > m_memoryLimit && parentNode->child()[ 0 ].isLeaf() )
// 		{
// 			#ifdef ASYNC_LOAD
// 				m_nodeLoader.asyncRelease( std::move( parentNode->child() ), omp_get_thread_num() );
// 			#else
// 				parentNode->releaseChildren();
// 			#endif
// 		}
// 		else
// 		{
			if( GpuAllocStatistics::reachedGpuMemQuota() )
			{
				for( Node& node : parentNode->child() )
				{
					#ifdef ASYNC_LOAD
						unloadInGpu( node );
					#else
						node.unloadGPU();
					#endif
				}
			}
// 		}
		
		FrontNode frontNode( *parentNode, parentMorton );
		
		if( parentIsCullable )
		{
			m_front.insert( frontIt, frontNode );
		}
		else
		{
			setupNodeRendering( frontIt, frontNode, renderer );
		}
	}
	
	template< typename Morton >
	inline bool Front< Morton >
	::checkBranch( const OctreeDim& nodeLvlDim, Node& node, const Morton& morton, Renderer& renderer,
				   const Float projThresh, bool& out_isCullable )
	{
		#ifdef BRANCHING_DEBUG
		{
			stringstream ss; ss << morton.getPathToRoot() << ": checking branch." << endl << node << endl << endl;
			HierarchyCreationLog::logDebugMsg( ss.str() );
		}
		#endif
		
		AlignedBox3f box = nodeLvlDim.getMortonBoundaries( morton );
		out_isCullable = renderer.isCullable( box );
		
		if( !node.isLeaf() && !node.child().empty() )
		{
			NodeArray& children = node.child();
			
			bool areChildrenLoaded = true;
			
			for( Node& child : children )
			{
				if( !child.isLoaded() )
				{
					#ifdef ASYNC_LOAD
						child.loadInGpu();
					#else
						child.loadGPU();
					#endif
					
					areChildrenLoaded = false;
				}
			}
			
			if( areChildrenLoaded )
			{
				return !renderer.isRenderable( box, projThresh ) && !out_isCullable;
			}
		}
		
		return false;
	}
	
	template< typename Morton >
	inline void Front< Morton >::branch( FrontListIter& frontIt, Node& node,
										 const OctreeDim& nodeLvlDim, Renderer& renderer )
	{
		#ifdef RENDERING_DEBUG
		{
			stringstream ss; ss << "Trying to remove from rendering (branch): " << frontIt->m_morton.getPathToRoot() << endl << endl;
			HierarchyCreationLog::logDebugMsg( ss.str() );
		}
		#endif
		
		frontIt = m_front.erase( frontIt );
		
		renderer.eraseFromList( node );
		
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
				setupNodeRendering( frontIt, frontNode, renderer );
			}
			else
			{
				m_front.insert( frontIt, frontNode );
			}
		}
	}
	
	template< typename Morton >
	inline void Front< Morton >::setupNodeRendering( FrontListIter& frontIt, const FrontNode& frontNode,
													 Renderer& renderer )
	{
		m_front.insert( frontIt, frontNode );
		
		setupNodeRenderingNoFront( frontNode.m_morton, *frontNode.m_octreeNode, renderer );
	}
	
	template< typename Morton >
	inline void Front< Morton >::setupNodeRenderingNoFront( const Morton& morton, Node& node, Renderer& renderer )
	{
		if( node.isLoaded() )
		{
			#ifdef NODE_ID_TEXT
			{
				const Vec3& textPos = node.getContents()[ 0 ].c;
				
				m_nodeIds.push_back(
					pair< string, Vector4f >( morton.toString(), Vector4f( textPos.x(), textPos.y(), textPos.z(), 1.f ) )
				);
			}
			#endif
		
			#if CUT_RENDERING_METHOD == RENDER_NEW_CUT_ONLY
				if( m_lastRendered < morton )
				{
					#ifdef TUCANO_RENDERER
						renderer.render_cloud_tucano( node );
					#else
						renderer.render( node );
					#endif
					
					m_lastRendered = morton;
				}
			#elif CUT_RENDERING_METHOD == RENDER_OLD_CUT_ONLY
				if( m_lastRendered < morton )
				{
					m_lastRendered = morton;
				}
				else
				{
					#ifdef TUCANO_RENDERER
						renderer.render_cloud_tucano( node );
					#else
						renderer.render( node );
					#endif
				}
			#else
				#ifdef TUCANO_RENDERER
					renderer.render_cloud_tucano( node );
				#else
					#ifdef RENDERING_DEBUG
					{
						stringstream ss; ss << "Rendering: " << morton.getPathToRoot() << endl << endl;
						HierarchyCreationLog::logDebugMsg( ss.str() );
					}
					#endif
					
					renderer.render( node );
				#endif
			#endif
		}
	}
	
	template< typename Morton >
	inline void Front< Morton >::unloadInGpu( Node& node )
	{
		for( Node& child : node.child() )
		{
			if( child.isLoaded() )
			{
				child.unloadInGpu();
			}
		}
		
		node.unloadInGpu();
	}
}

#undef INSERTION_DEBUG
#undef SUBSTITUTION_DEBUG
#undef ORDERING_DEBUG
#undef RENDERING_DEBUG
#undef FRONT_TRACKING_DEBUG
#undef PRUNING_DEBUG
#undef BRANCHING_DEBUG

#undef ASYNC_LOAD
#undef NODE_ID_TEXT

#endif