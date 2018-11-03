#ifndef FRONT_H
#define FRONT_H

#include <list>
#include "omicron/hierarchy/o1_octree_node.h"
#include "omicron/hierarchy/octree_dimensions.h"
#include "omicron/hierarchy/octree_stats.h"
// #include "renderers/StreamingRenderer.h"
#include "omicron/renderer/splat_renderer/splat_renderer.hpp"
#include "omicron/util/profiler.h"
#include "omicron/util/stack_trace.h"
#include "tucano/effects/phongshader.hpp"
#include "omicron/hierarchy/hierarchy_creation_log.h"
#include "omicron/renderer/text_effect.h"
#include "omicron/memory/global_malloc.h"

// Definitions to turn on debug logging for each Front operation.
// #define INSERTION_DEBUG
// #define ORDERING_DEBUG
// #define RENDERING_DEBUG
// #define FRONT_TRACKING_DEBUG
// #define PRUNING_DEBUG
// #define BRANCHING_DEBUG

// Turn on asynchronous GPU node loading.
#define ASYNC_LOAD

// Cut rendering methods.
#define RENDER_ENTIRE_CUT 0 // Renders the entire cut. The default and correct method to visualize the point cloud.
#define RENDER_OLD_CUT_ONLY 1 // Debug. Renders only nodes that were in cuts rendered already.
#define RENDER_NEW_CUT_ONLY 2 // Debug. Renders only nodes at the end of thesubstitutePlaceholder front that were added to the cut recently
							  // and were not rendered yet.

// Choosen cut rendering method
#define CUT_RENDERING_METHOD RENDER_ENTIRE_CUT

// Render an id text in each rendered node.
// #define NODE_ID_TEXT

namespace omicron::hierarchy
{
    using namespace std;
    using namespace util;
	using namespace renderer;
    
	/** Visualization front of an hierarchy under construction. Front ensures that its nodes are always sorted
	 * in hierarchy's width and that nodes can be inserted in a multithreaded environment. The nodes must be inserted in
	 * iterations. For a given iteration, the insertion threads can parallely insert nodes of a continuous front segment,
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
		using Node = O1OctreeNode< Morton >;
		using NodeArray = Array< Node >;
		using OctreeDim = OctreeDimensions< Morton >;
		using Renderer = SplatRenderer< Morton >;
		
		/** The node type that is used in front. */
		/* typedef struct FrontNode
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
		} FrontNode; */
		
		using FrontList = list< Node*, ManagedAllocator< Node* > >;
		using FrontListIter = typename FrontList::iterator;
		using InsertionVector = vector< FrontList, ManagedAllocator< FrontList > >;
		
		/** Ctor.
		 * @param dbFilename is the path to a database file which will be used to store nodes in an out-of-core approach.
		 * @param leafLvlDim is the information of octree size at the deepest (leaf) level. */
		Front( const string& dbFilename, const OctreeDim& leafLvlDim, const int nHierarchyCreationThreads, const ulong memoryLimit );
		
		/** Inserts a node into thread's buffer end so it can be push to the front later on. After this, the tracking
		 * method will ensure that the placeholder related with this node, if any, will be substituted by it.
		 * @param node is a reference to the node to be inserted.
		 * @param morton is node's morton code id.
		 * @param threadIdx is the hierarchy creation thread index of the caller thread. */
		void insertIntoBufferEnd( Node& node, int threadIdx );
		
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
		void insertIntoBuffer( FrontListIter& iter, Node& node, int threadIdx );
		
		/** Synchronized. Notifies that all threads have finished an insertion iteration.
		 * @param dispatchedThreads is the number of dispatched thread in the creation iteration. */
		void notifyInsertionEnd( uint dispatchedThreads );
		
		/** THIS IS A HACK FOR TESTING PURPOSES. Inserts the root of the hierarchy into the front. In other words, makes
		 * the classic front initialization and ignores the bottom-up insertion capability provided in this class. Obviously,
		 * the hierarchy should be constructed beforehand and this should be the only insertion needed into the front. */
		void insertRoot( Node& root );
		
		/** Notifies that all leaf level nodes are already loaded. */
		void notifyLeafLvlLoaded();
		
		/** Tracks the front based on the projection threshold.
		 * @param renderer is the responsible of rendering the points of the tracked front.
		 * @param projThresh is the projection threashold */
		OctreeStats trackFront( Renderer& renderer, const Float projThresh );
		
		uint insertedLeaves() { return m_insertedLeaves; }

	private:
		void trackNode( FrontListIter& frontIt, Node*& lastParent, Renderer& renderer, const Float projThresh );
		
		bool checkPrune( const Morton& parentMorton, Node* parentNode, const OctreeDim& parentLvlDim,
						 FrontListIter& frontIt, Renderer& renderer,
					const Float projThresh, bool& out_isCullable );
		
		void prune( FrontListIter& frontIt, Node* parentNode, const bool parentIsCullable, Renderer& renderer );
		
		bool checkBranch( const OctreeDim& nodeLvlDim, Node& node, const Morton& morton, Renderer& renderer,
						  const Float projThresh, bool& out_isCullable );
		
		void branch( FrontListIter& iter, Node& node, const OctreeDim& nodeLvlDim, Renderer& renderer );
		
		void setupNodeRendering( FrontListIter& iter, Node* frontNode, Renderer& renderer );
		
		void setupNodeRenderingNoFront( const Morton& moton, Node& node, Renderer& renderer );
		
		void unloadInGpu( Node& node );
		
		bool checkNode( const Node& node ) const;

		bool checkInsertion( const Node& toInsert ) const;

		bool checkIterInsertions() const;
		
		string toString() const;

		/** The internal front datastructure. Contains all FrontNodes. */
		FrontList m_front;
		
		/** Iterator used to resume front processing from previous frame. */
		FrontListIter m_frontIter;
		
		/** Mutexes to synchronize m_perLvlInsertions operations. */
		mutex m_mutex;
		
		/** Nodes pending insertion in the current insertion iteration. m_currentIterInsertions[ t ] have the insertions
		 * of thread t. This lists are moved to m_perLvlInsertions whenever notifyInsertionEnd() is called. */
		InsertionVector m_currentIterInsertions;
		
		/** All pending insertion nodes. m_perLvlInsertions[ l ] have the nodes for level l, sorted in hierarchy width order. */
		FrontList m_insertions;
		
		/** Dimensions of the octree nodes at deepest level. */
		OctreeDim m_leafLvlDim;
		
		/** Memory usage limit. Used to turn on/off node release. */
		ulong m_memoryLimit;
		
		/** Indicates that all leaf level nodes are already loaded. */
		atomic_bool m_leafLvlLoadedFlag;
		
		// Statistics related data.
		chrono::system_clock::time_point m_lastInsertionTime;
		OctreeStats m_octreeStats;
		
		uint m_insertedLeaves;

		#ifdef NODE_ID_TEXT
			TextEffect m_textEffect;
			vector< pair< string, Vector4f >, TbbAllocator< pair< string, Vector4f > > > m_nodeIds;
		#endif
			
		#if CUT_RENDERING_METHOD == RENDER_NEW_CUT_ONLY || CUT_RENDERING_METHOD == RENDER_OLD_CUT_ONLY
			Morton m_lastRendered;
		#endif
	};
	
	template< typename Morton >
	inline Front< Morton >::Front( const string& dbFilename, const OctreeDim& leafLvlDim, const int nHierarchyCreationThreads, const ulong memoryLimit )
	: m_leafLvlDim( leafLvlDim ),
	m_memoryLimit( memoryLimit ),
	m_currentIterInsertions( nHierarchyCreationThreads ),
	m_leafLvlLoadedFlag( false ),
	m_lastInsertionTime( Profiler::now() ),
	m_insertedLeaves( 0u )
	{
		m_frontIter = m_front.end();
		
		#ifdef NODE_ID_TEXT
		{
			m_textEffect.initialize( "shaders/Inconsolata.otf" );
		}
		#endif
	}

	template< typename Morton >
	bool Front< Morton >::checkInsertion( const Node& toInsert ) const
	{
		assert( toInsert.getMorton().getLevel() == MAX_HIERARCHY_LVL );
		checkNode( toInsert );

		return true;
	}

	template< typename Morton >
	bool Front< Morton >::checkNode( const Node& node ) const
	{
		assert( Morton() < node.getMorton() );
		assert( node.getMorton() <= Morton::getLvlLast( MAX_HIERARCHY_LVL ) );

		return true;
	}

	template< typename Morton >
	bool Front< Morton >::checkIterInsertions() const
	{
		for( const FrontList& list : m_currentIterInsertions )
		{
			for( const Node* node : list )
			{
				checkInsertion( *node );
			}
		}

		return true;
	}

	template< typename Morton >
	void Front< Morton >::notifyLeafLvlLoaded()
	{
		m_leafLvlLoadedFlag = true;
	}
	
	template< typename Morton >
	inline void Front< Morton >::insertIntoBufferEnd( Node& node, int threadIdx )
	{
		#ifdef INSERTION_DEBUG
		{
			stringstream ss; ss << "Inserting " << node.getMorton().toString() << endl << &node << endl << endl;
			HierarchyCreationLog::logDebugMsg( ss.str() );
		}
		#endif
		
		//assert( node.getMorton().getBits() != 0x20fafe );

		assert( checkInsertion( node ) );

		FrontList& list = m_currentIterInsertions[ threadIdx ];
		list.push_back( &node );
	}
	
	template< typename Morton >
	inline typename Front< Morton >::FrontListIter Front< Morton >::getIteratorToBufferBegin( int threadIdx )
	{
		return m_currentIterInsertions[ threadIdx ].begin();
	}
	
	template< typename Morton >
	inline void Front< Morton >::insertIntoBuffer( FrontListIter& iter, Node& node, int threadIdx )
	{
		#ifdef INSERTION_DEBUG
		{
			stringstream ss; ss << "Inserting " << node.getMorton().toString() << endl << &node << endl << endl;
			HierarchyCreationLog::logDebugMsg( ss.str() );
		}
		#endif
		
		//assert( node.getMorton().getBits() != 0x20fafe );

		assert( checkInsertion( node ) );

		FrontList& list = m_currentIterInsertions[ threadIdx ];
		list.insert( iter, &node );
	}
	
	template< typename Morton >
	inline void Front< Morton >::notifyInsertionEnd( uint dispatchedThreads )
	{
		if( dispatchedThreads > 0 )
		{
			lock_guard< mutex > lock( m_mutex );
			
			assert( checkIterInsertions() );

			for( FrontList& list : m_currentIterInsertions )
			{
				// Move nodes to the per-level sorted buffer.
				m_insertions.splice( m_insertions.end(), list );
			}
		}
	}
	
	template< typename Morton >
	inline void Front< Morton >::insertRoot( Node& root )
	{
		assert( m_front.empty() && "Front should be empty before inserting root." );
		
		Morton rootCode; rootCode.build( 0x1 );
		m_front.push_back( FrontNode( root, rootCode ) );
		root.loadInGpu();
	}
	
	template< typename Morton >
	inline OctreeStats Front< Morton >::trackFront( Renderer& renderer, const Float projThresh )
	{
		auto start = Profiler::now();
		
		renderer.begin_frame();
		
		// Statistics.
		float frontInsertionDelay = 0.f;
		int nNodesPerFrame = 0;
		
		{
			lock_guard< mutex > lock( m_mutex );
			
			if( !m_insertions.empty() )
			{
				m_insertedLeaves += m_insertions.size();
				frontInsertionDelay = Profiler::elapsedTime( m_lastInsertionTime );
				m_lastInsertionTime = Profiler::now();
			}
				
			// Insert all leaf level placeholders.
			m_front.splice( m_front.end(), m_insertions );
		}
		
		if( !m_front.empty() )
		{
			Node* lastParent = nullptr; // Parent of last node. Used to optimize prunning check.
				
			#if defined FRONT_TRACKING_DEBUG || defined RENDERING_DEBUG
			{
				stringstream ss; ss << "===== FRONT TRACKING BEGINS =====" << endl << /*toString() <<*/ endl;
				HierarchyCreationLog::logDebugMsg( ss.str() );
			}
			#endif
			
			if( m_frontIter == m_front.end() )
			{
				m_frontIter = m_front.begin();
				renderer.resetIterator();
			}
			
			nNodesPerFrame = max( float( m_front.size() ) / float( SEGMENTS_PER_FRONT ), 1.f );
			
			for( int i = 0; m_frontIter != m_front.end() && i < nNodesPerFrame; ++i )
			{
				assert( checkNode( **m_frontIter ) );

				#ifdef FRONT_TRACKING_DEBUG
				{
					stringstream ss; ss << "Tracking " << ( *m_frontIter )->getMorton().getPathToRoot() << endl << *m_frontIter << endl << endl;
					HierarchyCreationLog::logDebugMsg( ss.str() );
				}
				#endif
				
				#ifdef ORDERING_DEBUG
					assertFrontIterator( m_frontIter, front );
				#endif
					
				trackNode( m_frontIter, lastParent, renderer, projThresh );
			}

			renderer.render_frame();

			#ifdef FRONT_TRACKING_DEBUG
			{
				HierarchyCreationLog::logDebugMsg( "===== FRONT TRACKING END =====\n\n" );
			}
			#endif
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
		
		int renderQueueTime = Profiler::elapsedTime( start );
		
		m_octreeStats.addFrame( FrameStats( traversalTime, renderQueueTime, numRenderedPoints, frontInsertionDelay, m_front.size(), nNodesPerFrame ) );
		
		return m_octreeStats;
	}
	
	template< typename Morton >
	inline void Front< Morton >
	::trackNode( FrontListIter& frontIt, Node*& lastParent, Renderer& renderer, const Float projThresh )
	{
		Node* frontNode = *frontIt;
		
		Node& node = *frontNode;
		const Morton& morton = frontNode->getMorton();
		OctreeDim nodeLvlDim( m_leafLvlDim, morton.getLevel() );
		
		Node* parentNode = node.parent();
		
		// If parentNode == lastParent, prunning was not sucessful for a sibling of the current node, so the prunning
		// check can be skipped.
		if( parentNode != nullptr && parentNode != lastParent )  
		{
			OctreeDim parentLvlDim( nodeLvlDim, nodeLvlDim.m_nodeLvl - 1 );
			Morton parentMorton = *morton.traverseUp();
			bool parentIsCullable;
			if( checkPrune( parentMorton, parentNode, parentLvlDim, frontIt, renderer,
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
				stringstream ss; ss << "Culling: " << morton.toString() << endl << endl;
				HierarchyCreationLog::logDebugMsg( ss.str() );
			}
			#endif
			
			renderer.eraseFromList( node );
		}
		
		frontIt++;
	}
	
	template< typename Morton >
	inline bool Front< Morton >
	::checkPrune( const Morton& parentMorton, Node* parentNode, const OctreeDim& parentLvlDim,
				  FrontListIter& frontIt, Renderer& renderer, const Float projThresh,
			   bool& out_isCullable )
	{
		#ifdef PRUNING_DEBUG
		{
			stringstream ss; ss << parentMorton.toString() << " checking prune." << endl << parentNode << endl << endl;
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
				stringstream ss; ss << parentMorton.toString() << ": CULLABLE." << endl << endl;
				HierarchyCreationLog::logDebugMsg( ss.str() );
			}
			#endif
		}
		else
		{
			#ifdef PRUNING_DEBUG
 			{
 				stringstream ss; ss << parentMorton.toString() << ": NOT CULLABLE." << endl << endl;
 				HierarchyCreationLog::logDebugMsg( ss.str() );
 			}
			#endif
			
			if( renderer.isRenderable( parentBox, projThresh ) )
			{
				#ifdef PRUNING_DEBUG
 				{
 					stringstream ss; ss << parentMorton.toString() << ": RENDERABLE." << endl << endl;
 					HierarchyCreationLog::logDebugMsg( ss.str() );
 				}
				#endif
				
				pruneFlag = true;
			}
			#ifdef PRUNING_DEBUG
 			else
 			{
 				stringstream ss; ss << parentMorton.toString() << ": NOT RENDERABLE." << endl << endl;
 				HierarchyCreationLog::logDebugMsg( ss.str() );
 			}
			#endif
		}
		
		if( pruneFlag )
		{
			int nSiblings = 0;
			FrontListIter siblingIter = frontIt;
			while( siblingIter != m_front.end() )
			{	
				if( ( *siblingIter++ )->parent() != parentNode )
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
					stringstream ss; ss << parentMorton.toString() << ": last sibling group." << endl << endl;
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
 			{
 				stringstream ss; ss << parentMorton.toString() << ": not loaded." << endl << endl;
 				HierarchyCreationLog::logDebugMsg( ss.str() );
 			}
			#endif
				
			pruneFlag = false;
		}
		
		#ifdef PRUNING_DEBUG
 		{
 			stringstream ss; ss << parentMorton.toString() << ": prunning successful? " << pruneFlag << endl << endl;
 			HierarchyCreationLog::logDebugMsg( ss.str() );
 		}
		#endif
		
		return pruneFlag;
	}
	
	template< typename Morton >
	inline void Front< Morton >::prune( FrontListIter& frontIt, Node* parentNode, const bool parentIsCullable,
										Renderer& renderer )
	{
		Morton parentMorton = *( ( *frontIt )->getMorton().traverseUp() );
		
		assert( ( *frontIt )->parent() == parentNode );

		while( frontIt != m_front.end() && ( *frontIt )->parent() == parentNode )
		{
			#ifdef RENDERING_DEBUG
			{
				stringstream ss; ss << "Pruning: " << ( *frontIt )->getMorton().toString() << endl << endl;
				HierarchyCreationLog::logDebugMsg( ss.str() );
			}
			#endif
			
			renderer.eraseFromList( **frontIt );
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
		
		if( parentIsCullable )
		{
			m_front.insert( frontIt, parentNode );
		}
		else
		{
			setupNodeRendering( frontIt, parentNode, renderer );
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
			stringstream ss; ss << "Branch: " << ( *frontIt )->getMorton().toString() << endl << endl;
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
			
			if( !renderer.isCullable( box ) )
			{
				setupNodeRendering( frontIt, &child, renderer );
			}
			else
			{
				m_front.insert( frontIt, &child );
			}
		}
	}
	
	template< typename Morton >
	inline void Front< Morton >::setupNodeRendering( FrontListIter& frontIt, Node* frontNode,
													 Renderer& renderer )
	{
		m_front.insert( frontIt, frontNode );
		
		setupNodeRenderingNoFront( frontNode->getMorton(), *frontNode, renderer );
	}
	
	template< typename Morton >
	inline void Front< Morton >::setupNodeRenderingNoFront( const Morton& morton, Node& node, Renderer& renderer )
	{
		if( node.isLoaded() )
		{
			#ifdef NODE_ID_TEXT
			{
				const Vec3& textPos = ExtOctreeData::getSurfel( ExtOctreeData::getIndex( node.offset() ) ).c;
				
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

	template< typename Morton >
	inline string Front< Morton >::toString() const
	{
		stringstream ss; ss << "Front size: " << m_front.size() << endl;

		for( const Node* node : m_front )
		{
			ss << node->getMorton().toString() << endl;
		}

		return ss.str();
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
