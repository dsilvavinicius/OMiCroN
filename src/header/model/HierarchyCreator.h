#ifndef HIERARCHY_CREATOR_H
#define HIERARCHY_CREATOR_H

#include <mutex>
#include <fstream>
#include <condition_variable>
#include <future>
#include <signal.h>
#include "ManagedAllocator.h"
#include "O1OctreeNode.h"
#include "OctreeDimensions.h"
#include "Front.h"
#include "SQLiteManager.h"
#include "HierarchyCreationLog.h"

// Enables node colapse when leaves do not have siblings.
// #define NODE_COLAPSE

// #define LEAF_CREATION_DEBUG
// #define INNER_CREATION_DEBUG
// #define NODE_PROCESSING_DEBUG
// #define NODE_LIST_MERGE_DEBUG
// #define PARENT_DEBUG

#define PARENT_POINTS_RATIO 0.125f/*0.015625f*/

using namespace util;

namespace model
{
	// TODO: If this algorithm is the best one, change MortonCode API to get rid of shared_ptr.
	/** Multithreaded massive octree hierarchy creator. */
	template< typename Morton >
	class HierarchyCreator
	{
	public:
		using PointArray = Array< Surfel >;
		using PointVector = vector< Surfel, TbbAllocator< Surfel > >;
		using Node = O1OctreeNode< Surfel >;
		using NodeArray = Array< Node >;
		
		using OctreeDim = OctreeDimensions< Morton >;
		using Front = model::Front< Morton >;
		using Reader = PlyPointReader;
		//using Sql = SQLiteManager< Point, Morton, Node >;
		
		/** List of nodes that can be processed parallel by one thread. */
		using NodeList = list< Node, ManagedAllocator< Node > >;
		// List of NodeLists.
		using WorkList = list< NodeList, ManagedAllocator< Node > >;
		// Array with lists that will be processed in a given creation loop iteration.
		using IterArray = Array< NodeList >;
		
		/** Ctor.
		 * @param sortedPlyFilename is a sorted .ply point filename.
		 * @param dim is the OctreeDim of the octree to be constructed.
		 * @param expectedLoadPerThread is the size of the NodeList that will be passed to each thread in the
		 * hierarchy creation loop iterations.
		 * @param memoryLimit is the allowed soft limit of memory consumption by the creation algorithm. */
		HierarchyCreator( const string& sortedPlyFilename, const OctreeDim& dim, Front& front,
						  ulong expectedLoadPerThread, const ulong memoryLimit, int nThreads = 8 );
		
		/** Creates the hierarchy asychronously.
		 * @return a future that will contain the hierarchy's root node when done. The node pointer ownership is caller's.
		 */
		future< Node* > createAsync();
		
		#ifdef HIERARCHY_STATS
			atomic_ulong m_processedNodes;
		#endif
		
	private:
		/** Map type used to perform a prefix-sum in nodes of a sibling group. */
		using SiblingPointsPrefixMap = map< int, Node&, less< int >, ManagedAllocator< pair< const int, Node& > > >;
		/** Entry type of SiblingPointsPrefixMap. */
		using SiblingPointsPrefixMapEntry = typename SiblingPointsPrefixMap::value_type;
		
		/** Creates the hierarchy.
		 * @return hierarchy's root node. The pointer ownership is caller's. */
		Node* create();
		
		void pushWork( NodeList&& workItem );
		
		NodeList popWork( const int lvl );
		
		size_t updatedWorkListSize( int lvl );
		
		/** Sets the parent pointer for all children of a given node, inserting them into front's thread buffer if they
		 * are leaves.
		 * @param node is the node which children will have the parent pointer set.
		 * @param threadIdx is the index of the front's thread buffer which the node belongs to. */
		void setParent( Node& node, const int threadIdx ) /*const*/;
		
		/** Sets the parent pointer for all children of a given node, inserting them into front's thread buffer if they
		 * are leaves. In this version, an iterator to the thread buffer indicates where the node should be inserted into.
		 * The thread index should be to the same front segment of the iterator.
		 * @param node is the node which children will have the parent pointer set.
		 * @param threadIdx is the index of the thread front segment which the node belongs to.
		 * @param frontIter is the iterator to the segment index threadIdx. The node will be inserted before the iterator
		 * (the same way as STL). */
		void setParent( Node& node, const int threadIdx, typename Front::FrontListIter& frontIter ) /*const*/;
		
	#ifdef NODE_COLAPSE
		/** Collapses (turn into leaf) a node if it has only one child. */
		void collapse( Node& node ) const;
		
		/** If needed, collapses (turn into leaf) the boundary nodes of a worklist. */
		void collapseBoundaries( NodeList& list ) const;
	#endif
		
		/** If needed, removes the boundary duplicate node in previousProcessed, moving its children to nextProcessed.
		 * Boundary duplicates can occur if nodes from the same sibling group are processed in different threads. */
		void removeBoundaryDuplicate( NodeList& previousProcessed, const int previousIdx, NodeList& nextProcessed,
									  const OctreeDim& nextLvlDim )
		/*const*/;
		
		/** Merge previousProcessed into nextProcessed if there is not enough work yet to form a WorkList or push it to
		 * the next level WorkList otherwise. Repetitions are checked while linking lists, since it can occur when the
		 * lists have nodes from the same sibling group.
		 * @param previousProcessed is the previous WorkList.
		 * @param previousIdx is the thread index of the thread that computed previousProcessed.
		 * @param nextProcessed is the next WorkList.
		 * @param nextLvlDim has the dimensions of the octree for the level of the nodes in previousProcessed and
		 * nextProcessed. */
		void mergeOrPushWork( NodeList& previousProcessed, const int previousIdx, NodeList& nextProcessed,
							  OctreeDim& nextLvlDim );
		
		/** Creates a node from its solo child node. */
		Node createNodeFromSingleChild( Node&& child, bool isLeaf, const int threadIdx,
										const bool setParentFlag ) /*const*/;
		
		/** Creates an inner Node, given a children array and the number of meaningfull entries in the array. */
		Node createInnerNode( NodeArray&& inChildren, uint nChildren, const int threadIdx,
							  const bool setParentFlag ) /*const*/;
		
		/** Creates a point sample with 1/8 of the points in the prefix-sum map. */
		PointArray samplePoints( const SiblingPointsPrefixMap& prefixMap, const int nPoints ) const;
							  
		/** Checks if all work is finished in all lvls. */
		bool checkAllWorkFinished();
		
		void turnReleaseOn( mutex& releaseMutex, bool& isReleasing );
		
		/** Sets all data to ensure that the algorithm's release is turned off. */
		void turnReleaseOff( mutex& releaseMutex, bool& isReleasing, condition_variable& releaseFlag,
							 mutex& diskThreadMutex, bool& isDiskThreadStopped );
		
		/** Releases and persists a given sibling group.
		 * @param siblings is the sibling group to be released and persisted if it is the case.
		 * @param threadIdx is the index of the executing thread.
		 * @param dim is the dimensions of the octree for the sibling lvl. */
		//void releaseSiblings( NodeArray& siblings, const int threadIdx, const OctreeDim& dim );
		
		string nodeListToString( const NodeList& list, const OctreeDim& lvlDim );
		
		string workListToString( const WorkList& list, const OctreeDim& lvlDim );
		
		/** Thread[ i ] uses database connection m_dbs[ i ]. */
		//Array< Sql > m_dbs;
		
		/** Worklists for every level in the hierarchy. m_lvlWorkLists[ i ] corresponds to lvl i. */
		Array< WorkList > m_lvlWorkLists;
		
		/** Current lvl octree dimensions. */
		OctreeDim m_octreeDim;
		
		/** Octree dimensions of the leaf level in this octree. */
		OctreeDim m_leafLvlDim;
		
		string m_plyFilename;
		
		mutex m_listMutex;
		
		ulong m_memoryLimit;
		
		ulong m_expectedLoadPerThread;
		
		Front& m_front;
		
		int m_nThreads;
	};
	
	template< typename Morton >
	HierarchyCreator< Morton >
	::HierarchyCreator( const string& sortedPlyFilename, const OctreeDim& dim, Front& front, ulong expectedLoadPerThread,
						const ulong memoryLimit, int nThreads )
	: m_plyFilename( sortedPlyFilename ), 
	m_lvlWorkLists( dim.m_nodeLvl + 1 ),
	m_leafLvlDim( dim ),
	m_nThreads( nThreads ),
	m_expectedLoadPerThread( expectedLoadPerThread ),
	//m_dbs( nThreads ),
	m_memoryLimit( memoryLimit ),
	m_front( front )
	
	#ifdef HIERARCHY_STATS
		, m_processedNodes( 0 )
	#endif
	{
		srand( 1 );
		omp_set_num_threads( m_nThreads );
	}
	
	template< typename Morton >
	future< typename HierarchyCreator< Morton >::Node* > HierarchyCreator< Morton >::createAsync()
	{
		packaged_task< Node*() > task(
			[ & ]
			{
				auto start = Profiler::now( "Hierarchy creation" );
				
				Node* root = create();
				
				Profiler::elapsedTime( start, "Hierarchy creation" );
				
				return root;
			}
		);
		auto future = task.get_future();
		thread t( std::move( task ) );
		t.detach();
		
		return future;
	}
	
	template< typename Morton >
	typename HierarchyCreator< Morton >::Node* HierarchyCreator< Morton >::create()
	{
		cout << "MEMORY BEFORE CREATING: " << AllocStatistics::totalAllocated() << endl << endl;
		
		// SHARED. The disk access thread sets this true when it finishes reading all points in the sorted file.
		bool leafLvlLoaded = false;
		
		// SHARED. Release flags.
		mutex releaseMutex;
		bool isReleasing = false;
		condition_variable releaseFlag;
		
		// SHARED. Indicates the disk thread is stopped.
		mutex diskThreadMutex;
		bool isDiskThreadStopped = false;
		
		// Thread that loads data from sorted file or database.
		thread diskAccessThread(
			[ & ]()
			{
				OctreeDim leafLvlDimCpy = m_leafLvlDim; // Just to make sure it will be in thread local mem.
				NodeList nodeList;
				PointVector points;
				
				Morton currentParent;
				Reader reader( m_plyFilename );
				reader.read(
					[ & ]( const Point& p )
					{
// 						Point p( point );
// 						
// 						// A shift in z direction in order to avoid points lying in z = 0 plane. Necessary because
// 						// of later projection operations, performed in homogeneous coordinates. 
// 						p.getPos() += Vec3( 0.f, 0.f, 1.f );
						
						Morton code = leafLvlDimCpy.calcMorton( p );
						Morton parent = *code.traverseUp();
						
						if( parent != currentParent )
						{
							if( points.size() > 0 )
							{
								#ifdef LEAF_CREATION_DEBUG
								{
									stringstream ss; ss << "Creating node "
										<< leafLvlDimCpy.calcMorton( points[ 0 ] ).getPathToRoot() << endl << endl;
									HierarchyCreationLog::logDebugMsg( ss.str() );
								}
								#endif
								
								
								nodeList.push_back( Node( std::move( points ), true ) );
								
								points = PointVector();
								
								if( nodeList.size() == m_expectedLoadPerThread )
								{
									pushWork( std::move( nodeList ) );
									nodeList = NodeList();
									
									bool isReleasingCpy;
									{
										lock_guard< mutex > lock( releaseMutex );
										isReleasingCpy = isReleasing;
									}
									
									if( isReleasingCpy )
									{
										{
											lock_guard< mutex > lock( diskThreadMutex );
											isDiskThreadStopped = true;
										}
							
										unique_lock< mutex > lock( releaseMutex );
										releaseFlag.wait( lock, [ & ] { return !isReleasing; } );
									}
								}
							}
							currentParent = parent;
						}
						
						const Vec3& pos = p.getPos();
						const Vec3& normal = p.getNormal();
						
						// This can lead to a division by 0.
						float epsilon = 1e-10;
						float planeMinusD = normal.x() * pos.x() + normal.y() * pos.y() + normal.z() * pos.z();
						
						Vec3 pointOnPlane;
						if( fabs( normal.x() ) > epsilon )
						{
							pointOnPlane = Vec3( planeMinusD / normal.x(), 0.f, 0.f );
						}
						else if( fabs( normal.y() ) > epsilon )
						{
							pointOnPlane = Vec3( 0.f, planeMinusD / normal.y(), 0.f );
						}
						else if( fabs( normal.z() ) > epsilon )
						{
							pointOnPlane = Vec3( 0.f, 0.f, planeMinusD / normal.z() );
						}
						else
						{
// 							cout << "Read point has zero vector normal." << endl << endl;
							return;
						}
						
						Vector3f u = pointOnPlane - pos;
						u.normalize();
						Vector3f v = normal.cross( u );
			
// 						u *= 0.00055f;
// 						v *= 0.00055f;
// 						u *= 0.00005f;
// 						v *= 0.00005f;
						u *= 0.000035f;
						v *= 0.000035f;
// 						u *= 0.00001f;
// 						v *= 0.00001f;
						
						points.push_back( Surfel( pos, u, v ) );
					}
				);
				
				nodeList.push_back( Node( std::move( points ), true ) );
				pushWork( std::move( nodeList ) );
				
				leafLvlLoaded = true;
				m_front.notifyLeafLvlLoaded();
				
				cout << "===== Leaf lvl loaded =====" << endl << endl;
			}
		);
		diskAccessThread.detach();
		
		// BEGIN MULTIPASS CONSTRUCTION LOOP.
		while( !leafLvlLoaded || !checkAllWorkFinished() )
		{
			bool isLastPass = leafLvlLoaded;
			m_octreeDim = m_leafLvlDim;
			int lvl = m_leafLvlDim.m_nodeLvl;
			bool nextPassFlag = false; // Indicates that the current algorithm pass should end and a new one should be issued.
			
			if( isReleasing && !m_front.isReleasing() )
			{
				turnReleaseOff( releaseMutex, isReleasing, releaseFlag, diskThreadMutex, isDiskThreadStopped );
			}
			
			// BEGIN HIERARCHY CONSTRUCTION LOOP.
			while( lvl )
			{
				OctreeDim nextLvlDim( m_octreeDim, m_octreeDim.m_nodeLvl - 1 );
				
				bool increaseLvlFlag = false;
				
				size_t workListSize = updatedWorkListSize( lvl );
				
				// BEGIN HIERARCHY LEVEL LOOP.
				while( workListSize > 0 && !increaseLvlFlag )
				{
					#ifdef NODE_PROCESSING_DEBUG
					{
						stringstream ss; ss << "===== Processing lvl " << lvl << " =====" << endl << endl; 
						
						HierarchyCreationLog::logDebugMsg( ss.str() );
					}
					#endif
					
					// Multipass restriction: the level's last sibling group cannot be processed until the last pass,
					// since nodes loaded after or in the middle of current pass can have remainings of that sibling group.
					// In the leaf lvl, the entire last NodeList is spared to avoid order issues generated by concurrent
					// work loading by the disk access thread.
					int dispatchedThreads;
					if( workListSize > m_nThreads )
					{
						dispatchedThreads = m_nThreads;
					}
					else
					{
						bool isDiskThreadStoppedCpy;
						{
							lock_guard< mutex > lock( diskThreadMutex );
							isDiskThreadStoppedCpy = isDiskThreadStopped;
						}
						if( lvl != m_leafLvlDim.m_nodeLvl || isLastPass || isDiskThreadStoppedCpy )
						{
							dispatchedThreads = workListSize;
							increaseLvlFlag = true;
						}
						else
						{
							dispatchedThreads = workListSize - 1;
							increaseLvlFlag = true;
						}
					}
					
					int lastThreadIdx = dispatchedThreads - 1;
					IterArray iterInput( dispatchedThreads );
					
					for( int i = 0; i < dispatchedThreads; ++i )
					{
						iterInput[ i ] = popWork( lvl );
					}
					
					IterArray iterOutput( dispatchedThreads );
					
					// BEGIN PARALLEL WORKLIST PROCESSING.
					#pragma omp parallel for
					for( int i = 0; i < dispatchedThreads; ++i )
					{
						int threadIdx = omp_get_thread_num();
						NodeList& input = iterInput[ threadIdx ];
						NodeList& output = iterOutput[ threadIdx ];
						bool isBoundarySiblingGroup = true;
						
						while( !input.empty() )
						{
							Node& node = input.front();
							Morton parentCode = *m_octreeDim.calcMorton( node ).traverseUp();
							
							NodeArray siblings( 8 );
							siblings[ 0 ] = std::move( node );
							input.pop_front();
							int nSiblings = 1;
							
							while( !input.empty() && *m_octreeDim.calcMorton( input.front() ).traverseUp() == parentCode )
							{
								siblings[ nSiblings ] = std::move( input.front() );
								++nSiblings;
								input.pop_front();
							}
							
							#ifdef NODE_PROCESSING_DEBUG
							{
								stringstream ss;
								for( int i = 0; i < nSiblings; ++i )
								{
									ss << "[ t" << omp_get_thread_num() << " ] processing: "
										<< m_octreeDim.calcMorton( siblings[ i ] ).getPathToRoot() << endl << endl;
								}
								
								HierarchyCreationLog::logDebugMsg( ss.str() );
							}
							#endif
							
							bool isLastSiblingGroup = input.empty();
							
							if( workListSize - dispatchedThreads == 0 && !isLastPass && threadIdx == lastThreadIdx
								&& isLastSiblingGroup )
							{
								// Send this last sibling group to the lvl WorkList again.
								NodeList lastSiblingsList;
								for( int j = 0; j < nSiblings; ++j )
								{
									lastSiblingsList.push_back( std::move( siblings[ j ] ) );
								}
								m_lvlWorkLists[ lvl ].push_back( std::move( lastSiblingsList ) );
							}
							else
							{
								if( isLastSiblingGroup )
								{
									isBoundarySiblingGroup = true;
								}
								
								if( nSiblings == 1 && siblings[ 0 ].isLeaf() && !isBoundarySiblingGroup )
								{
									#ifdef INNER_CREATION_DEBUG
									{
										stringstream ss; ss << "[ t" << omp_get_thread_num()
											<< " ] creating collapsed inner " << ( ( isBoundarySiblingGroup ) ? "boundary:"
											: "not boundary:" )
											<< nextLvlDim.calcMorton( siblings[ 0 ] ).getPathToRoot() << endl << endl;
										HierarchyCreationLog::logDebugMsg( ss.str() );
									}
									#endif
									
									#ifdef NODE_COLAPSE
									bool newNodeIsLeafFlag = true;
									#else
									bool newNodeIsLeafFlag = false;
									#endif
									
									output.push_back(
										createNodeFromSingleChild( std::move( siblings[ 0 ] ), newNodeIsLeafFlag,
																   threadIdx, !isBoundarySiblingGroup )
									);
								}
								else
								{
									#ifdef INNER_CREATION_DEBUG
									{
										stringstream ss; ss << "[ t" << omp_get_thread_num()
											<< " ] creating LoD inner " << ( ( isBoundarySiblingGroup ) ? "boundary:"
											: "not boundary:" )
											<< nextLvlDim.calcMorton( siblings[ 0 ] ).getPathToRoot() << endl << endl;
										HierarchyCreationLog::logDebugMsg( ss.str() );
									}
									#endif
									
									// LOD
									Node inner = createInnerNode( std::move( siblings ), nSiblings,
																  threadIdx, !isBoundarySiblingGroup );
									
									output.push_back( std::move( inner ) );
									isBoundarySiblingGroup = false;
								}
							}
						}
					}
					// END PARALLEL WORKLIST PROCESSING.
					
					WorkList& nextLvlWorkList = m_lvlWorkLists[ lvl - 1 ];
					
					if( !iterOutput.empty() && !iterOutput[ 0 ].empty() )
					{
						if( !nextLvlWorkList.empty() )
						{
							#ifdef NODE_LIST_MERGE_DEBUG
							{
								HierarchyCreationLog::logDebugMsg( "Merging previous work list with t 0 ouput\n\n" );
							}
							#endif
							
							NodeList nextLvlBack = std::move( nextLvlWorkList.back() );
							nextLvlWorkList.pop_back();
							
							mergeOrPushWork( nextLvlBack, -1, iterOutput[ 0 ], nextLvlDim );
						}
						else if( iterOutput[ 0 ].size() > 1 )
						{
							#ifdef NODE_LIST_MERGE_DEBUG
							{
								HierarchyCreationLog::logDebugMsg( "Setting parent of first node in t 0 ouput\n\n" );
							}
							#endif
							
							// Setup the parent for the first node in thread[ 0 ] output. This is
							// necessary because it won't be merged with a previous workList.
							Node& firstNode = iterOutput[ 0 ].front();							
							auto iter = m_front.getIteratorToBufferBegin( 0 );
							for( Node& child : firstNode.child() )
							{
								setParent( child, 0, iter );
							}
						}
					}
					
					for( int i = 0; i < lastThreadIdx; ++i )
					{
						#ifdef NODE_LIST_MERGE_DEBUG
						{
							stringstream ss; ss << "Merging t " << i << " with t " << i + 1 << endl << endl;
							HierarchyCreationLog::logDebugMsg( ss.str() );
						}
						#endif
						
						mergeOrPushWork( iterOutput[ i ], i, iterOutput[ i + 1 ], nextLvlDim );
					}
					
					// The last thread's NodeList is not collapsed, since the last node can be in a sibling group not
					// entirely processed in this iteration.
					if( !iterOutput.empty() && !iterOutput[ lastThreadIdx ].empty() )
					{
						if( !nextLvlWorkList.empty() )
						{
							removeBoundaryDuplicate( nextLvlWorkList.back(), lastThreadIdx, iterOutput[ lastThreadIdx ],
													 nextLvlDim );
						}
						nextLvlWorkList.push_back( std::move( iterOutput[ lastThreadIdx ] ) );
					}
					
					m_front.notifyInsertionEnd( dispatchedThreads );
					// END LOAD BALANCE.
					
					// BEGIN NODE RELEASE MANAGEMENT.
					if( isReleasing )
					{
						if( AllocStatistics::totalAllocated() < m_memoryLimit )
						{
							turnReleaseOff( releaseMutex, isReleasing, releaseFlag, diskThreadMutex, isDiskThreadStopped );
						}
					}
					else if( AllocStatistics::totalAllocated() > m_memoryLimit )
					{
						turnReleaseOn( releaseMutex, isReleasing );
					}
					// END NODE RELEASE MANAGEMENT.
					
					workListSize = updatedWorkListSize( lvl );
					
					size_t leafLvlWorkCount = ( lvl == m_leafLvlDim.m_nodeLvl ) ? workListSize :
						updatedWorkListSize( m_leafLvlDim.m_nodeLvl );
					
					if( !isLastPass && workListSize < m_nThreads &&
						m_lvlWorkLists[ lvl - 1 ].size() < leafLvlWorkCount )
					{
						// There is more work available in the deeper levels. Issue a new algorithm pass.
						nextPassFlag = true;
						break;
					}
				}
				// END HIERARCHY LEVEL LOOP.
				
				if( nextPassFlag || ( workListSize == 0 && !isLastPass ) )
				{
					break;
				}
				else
				{
					if( isLastPass )
					{
						// The last NodeList is not collapsed and the parent of its children is not set yet.
						NodeList& lastList = m_lvlWorkLists[ lvl - 1 ].back();
						
						#ifdef NODE_COLAPSE
							collapseBoundaries( lastList );
						#endif
						
						for( Node& child : lastList.back().child() )
						{
							setParent( child, 0 );
						}
						
						m_front.notifyInsertionEnd( 1 );
					}
					
					--lvl;
					m_octreeDim = OctreeDim( m_octreeDim, lvl );
				}
			}
			// END HIERARCHY CONSTRUCTION LOOP.
			
// 			++itersAfterRelease;
		}
		// END MULTIPASS CONSTRUCTION LOOP.
		
		Node* root = new Node( std::move( m_lvlWorkLists[ 0 ].front().front() ) );
		
		for( Node& child : root->child() )
		{
			setParent( child, 0 );
		}
		m_front.notifyInsertionEnd( 1 );
		
		setParent( *root, 0 );
		m_front.notifyInsertionEnd( 1 );
		
		if( root->isLeaf() )
		{
			Morton rootCode; rootCode.build( 0x1 );
			m_front.insertIntoBufferEnd( *root, rootCode, 0 );
			m_front.notifyInsertionEnd( 1 );
		}
		
		return root;
	}
	
	template< typename Morton >
	inline void HierarchyCreator< Morton >::pushWork( NodeList&& workItem )
	{
		lock_guard< mutex > lock( m_listMutex );
		
		m_lvlWorkLists[ m_leafLvlDim.m_nodeLvl ].push_back( std::move( workItem ) );
	}
	
	template< typename Morton >
	inline typename HierarchyCreator< Morton >::NodeList HierarchyCreator< Morton >
	::popWork( const int lvl )
	{
		if( lvl == m_leafLvlDim.m_nodeLvl )
		{
			lock_guard< mutex > lock( m_listMutex );
			NodeList nodeList = std::move( m_lvlWorkLists[ lvl ].front() );
			m_lvlWorkLists[ lvl ].pop_front();
			return nodeList;
		}
		else
		{
			NodeList nodeList = std::move( m_lvlWorkLists[ lvl ].front() );
			m_lvlWorkLists[ lvl ].pop_front();
			return nodeList;
		}
	}
	
	template< typename Morton >
	inline size_t HierarchyCreator< Morton >::updatedWorkListSize( int lvl )
	{
		if( lvl == m_leafLvlDim.m_nodeLvl )
		{
			// This lock is need so m_workList.size() access is not optimized, returning outdated results.
			lock_guard< mutex > lock( m_listMutex );
			return m_lvlWorkLists[ lvl ].size();
		}
		else
		{
			return m_lvlWorkLists[ lvl ].size();
		}
	}
		
	/** Sets the parent pointer for all children of a given node. */
	template< typename Morton >
	inline void HierarchyCreator< Morton >::setParent( Node& node, const int threadIdx ) /*const*/
	{
		// Parents are expected to be set once.
		if( !node.child().empty() && node.child()[ 0 ].parent() == nullptr )
		{
			#ifdef PARENT_DEBUG
			{
				stringstream ss; ss << "Setting as parent: " << m_octreeDim.calcMorton( node ).getPathToRoot()
					<< endl << endl;
				HierarchyCreationLog::logDebugMsg( ss.str() );
			}
			#endif
			
			OctreeDim childDim( m_octreeDim, m_octreeDim.m_nodeLvl + 1 );
			
			for( Node& child : node.child() )
			{
				child.setParent( &node );
				
				if( child.isLeaf() )
				{
					m_front.insertIntoBufferEnd( child, childDim.calcMorton( child ), threadIdx );
				}
			}
		}
	}
	
	template< typename Morton >
	void HierarchyCreator< Morton >::setParent( Node& node, const int threadIdx,
												typename Front::FrontListIter& frontIter )
	{
		// Parents are expected to be set once.
		if( !node.child().empty() && node.child()[ 0 ].parent() == nullptr )
		{
			#ifdef PARENT_DEBUG
			{
				stringstream ss; ss << "Setting as parent: " << m_octreeDim.calcMorton( node ).getPathToRoot()
					<< endl << endl;
				HierarchyCreationLog::logDebugMsg( ss.str() );
			}
			#endif
			
			OctreeDim childDim( m_octreeDim, m_octreeDim.m_nodeLvl + 1 );
			
			for( Node& child : node.child() )
			{
				child.setParent( &node );
				
				#ifdef DEBUG
// 				{
// 					cout << "Child: " << childDim.calcMorton( child ).getPathToRoot( true ) << child << endl << endl;
// 				}
				#endif
				
				if( child.isLeaf() )
				{
					m_front.insertIntoBuffer( frontIter, child, childDim.calcMorton( child ), threadIdx );
				}
			}
		}
	}
	
#ifdef NODE_COLAPSE
	template< typename Morton >
	inline void HierarchyCreator< Morton >::collapse( Node& node ) const
	{
		NodeArray& children = node.child();
		
		if( children.size() == 1 && children[ 0 ].isLeaf() )
		{
			node.turnLeaf();
		}
	}
	
	/** If needed, collapse (turn into leaf) the boundary nodes of a worklist. */
	template< typename Morton >
	inline void HierarchyCreator< Morton >::collapseBoundaries( NodeList& list )
	const
	{
		if( !list.empty() )
		{
			collapse( list.front() );
			collapse( list.back() );
		}
	}
#endif

	/** If needed, removes the boundary duplicate node in previousProcessed, moving its children to nextProcessed.
	 * Boundary duplicates can occur if nodes from the same sibling group are processed in different threads. */
	template< typename Morton >
	inline void HierarchyCreator< Morton >
	::removeBoundaryDuplicate( NodeList& previousProcessed, const int previousIdx, NodeList& nextProcessed,
							   const OctreeDim& nextLvlDim ) /*const*/
	{
		if( !previousProcessed.empty() && !nextProcessed.empty() )
		{
			Node& prevLastNode = previousProcessed.back();
			Node& nextFirstNode = nextProcessed.front();
			NodeArray& prevLastNodeChild = prevLastNode.child();
			NodeArray& nextFirstNodeChild = nextFirstNode.child();
			
			typename Front::FrontListIter it;
			if( previousIdx == -1 )
			{
				it = m_front.getIteratorToBufferBegin( 0 );
			}
			
			if( nextLvlDim.calcMorton( prevLastNode ) == nextLvlDim.calcMorton( nextFirstNode ) )
			{
				// Nodes from same sibling groups were in different threads
				
				int nPoints = 0;
				SiblingPointsPrefixMap prefixMap;
				NodeArray mergedChild( prevLastNodeChild.size() + nextFirstNodeChild.size() );
				
				for( int i = 0; i < prevLastNodeChild.size(); ++i )
				{
					mergedChild[ i ] = std::move( prevLastNodeChild[ i ] );
					Node& child = mergedChild[ i ];
					
					prefixMap.insert( prefixMap.end(), SiblingPointsPrefixMapEntry( nPoints, child ) );
					nPoints += child.getContents().size();
					
					if( previousIdx == -1 )
					{
						setParent( child, 0, it );
					}
					else
					{
						setParent( child, previousIdx );
					}
				}
				
				for( int i = 0; i < nextFirstNodeChild.size(); ++i )
				{
					int idx = prevLastNodeChild.size() + i;
					mergedChild[ idx ] = std::move( nextFirstNodeChild[ i ] );
					
					Node& child = mergedChild[ idx ];
					
					prefixMap.insert( prefixMap.end(), SiblingPointsPrefixMapEntry( nPoints, child ) );
					nPoints += child.getContents().size();
					
					if( previousIdx == -1 )
					{
						setParent( child, 0, it );
					}
					else
					{
						setParent( child, previousIdx );
					}
				}
				
				PointArray selectedPoints = samplePoints( prefixMap, nPoints );
				nextFirstNode.setContents( std::move( selectedPoints ) );
				nextFirstNode.setChildren( std::move( mergedChild ) );
				
				previousProcessed.pop_back();
			}
			else
			{
				// Setup parents for boundary nodes children.
				for( Node& child : prevLastNodeChild )
				{
					if( previousIdx == -1 )
					{
						setParent( child, 0, it );
					}
					else
					{
						setParent( child, previousIdx );
					}
				}
				
				for( Node& child : nextFirstNodeChild )
				{
					if( previousIdx == -1 )
					{
						setParent( child, 0, it );
					}
					else
					{
						setParent( child, previousIdx );
					}
				}
			}
			
			#ifdef NODE_COLAPSE
				collapseBoundaries( previousProcessed );
			
				// If needed, collapse the first node of nextProcessed
				NodeArray& newNextFirstNodeChild = nextFirstNode.child();
				if( newNextFirstNodeChild.size() == 1 && newNextFirstNodeChild[ 0 ].isLeaf() )
				{
					nextFirstNode.turnLeaf();
				}
			#endif
		}
	}
		
	/** Merge previousProcessed into nextProcessed if there is not enough work yet to form a WorkList or push it to
		* the next level WorkList otherwise. Repetitions are checked while linking lists, since it can occur when the
		* lists have nodes from the same sibling group. */
	template< typename Morton >
	inline void HierarchyCreator< Morton >
	::mergeOrPushWork( NodeList& previousProcessed, const int previousIdx, NodeList& nextProcessed, OctreeDim& nextLvlDim )
	{
		removeBoundaryDuplicate( previousProcessed, previousIdx, nextProcessed, nextLvlDim );
	
		if( previousProcessed.size() < m_expectedLoadPerThread )
		{
			nextProcessed.splice( nextProcessed.begin(), previousProcessed );
		}
		else
		{
			WorkList& workList = m_lvlWorkLists[ nextLvlDim.m_nodeLvl ];
			
			if( !workList.empty() )
			{
				removeBoundaryDuplicate( workList.back(), previousIdx, previousProcessed, nextLvlDim );
			}
			
			workList.push_back( std::move( previousProcessed ) );
		}
	}
	
	template< typename Morton >
	inline bool HierarchyCreator< Morton >::checkAllWorkFinished()
	{
		lock_guard< mutex > lock( m_listMutex );
		
		for( int i = 1; i < m_lvlWorkLists.size(); ++i )
		{
			if( !m_lvlWorkLists[ i ].empty() )
			{
				return false;
			}
		}
		
		return true;
	}
	
	template< typename Morton >
	inline void HierarchyCreator< Morton >::turnReleaseOn( mutex& releaseMutex, bool& isReleasing )
	{
		{
			lock_guard< mutex > lock( releaseMutex );
			isReleasing = true;
		}
	}
		
	template< typename Morton >
	inline void HierarchyCreator< Morton >
	::turnReleaseOff( mutex& releaseMutex, bool& isReleasing, condition_variable& releaseFlag, mutex& diskThreadMutex,
						bool& isDiskThreadStopped )
	{
		{
			lock_guard< mutex > lock( releaseMutex );
			isReleasing = false;
		}
		
		releaseFlag.notify_one();
		
		{
			lock_guard< mutex > lock( diskThreadMutex );
			isDiskThreadStopped = false;
		}
	}
	
	template< typename Morton >
	inline typename HierarchyCreator< Morton >::Node HierarchyCreator< Morton >
	::createNodeFromSingleChild( Node&& child, bool isLeaf, const int threadIdx, const bool setParentFlag ) /*const*/
	{
		// Setup a placeholder in front if the node is in the leaf level.
		Morton childMorton = m_octreeDim.calcMorton( child );
		if( childMorton.getLevel() == m_leafLvlDim.m_nodeLvl )
		{
			m_front.insertPlaceholder( childMorton, threadIdx );
		}
		
		const PointArray& childPoints = child.getContents();
		
		int numSamplePoints = std::max( 1.f, childPoints.size() * PARENT_POINTS_RATIO );
		PointArray selectedPoints( numSamplePoints );
		
		for( int i = 0; i < numSamplePoints; ++i )
		{
			int choosenIdx = rand() % childPoints.size();
			selectedPoints[ i ] = childPoints[ choosenIdx ];
		}
		
		Node node( std::move( selectedPoints ), isLeaf );
		if( !isLeaf )
		{
			NodeArray children( 1 );
			children[ 0 ] = std::move( child );
			node.setChildren( std::move( children ) );
			
			Node& finalChild = node.child()[ 0 ];
			if( setParentFlag && !finalChild.isLeaf() )
			{
				setParent( finalChild, threadIdx );
			}
		}
		
		return node;
	}
	
	template< typename Morton >
	inline typename HierarchyCreator< Morton >::Node HierarchyCreator< Morton >
	::createInnerNode( NodeArray&& inChildren, uint nChildren, const int threadIdx, const bool setParentFlag ) /*const*/
	{
		if( nChildren == 1 )
		{
			return createNodeFromSingleChild( std::move( inChildren[ 0 ] ), false, threadIdx, setParentFlag );
		}
		else
		{
			NodeArray children( nChildren );
			
			// Verify if placeholders are necessary.
			bool frontPlaceholdersOn = ( m_octreeDim.calcMorton( inChildren[ 0 ] ).getLevel() == m_leafLvlDim.m_nodeLvl );
			
			int nPoints = 0;
			SiblingPointsPrefixMap prefixMap;
			for( int i = 0; i < children.size(); ++i )
			{
				children[ i ] = std::move( inChildren[ i ] );
				
				if( frontPlaceholdersOn )
				{
					m_front.insertPlaceholder( m_octreeDim.calcMorton( children[ i ] ), threadIdx );
				}
				
				Node& child = children[ i ];
				
				prefixMap.insert( prefixMap.end(), SiblingPointsPrefixMapEntry( nPoints, child ) );
				nPoints += child.getContents().size();
				
				// Set parental relationship of children.
				if( setParentFlag && !child.isLeaf() )
				{
					setParent( child, threadIdx );
				}
			}
			
			PointArray selectedPoints = samplePoints( prefixMap, nPoints );
			
			Node node( std::move( selectedPoints ), false );
			node.setChildren( std::move( children ) );
			
			return node;
		}
	}
	
	template< typename Morton >
	inline typename HierarchyCreator< Morton >::PointArray HierarchyCreator< Morton >
	::samplePoints( const SiblingPointsPrefixMap& prefixMap, const int nPoints ) const
	{
		int numSamplePoints = std::max( 1.f, nPoints * PARENT_POINTS_RATIO );
		PointArray selectedPoints( numSamplePoints );
		
		for( int i = 0; i < numSamplePoints; ++i )
		{
			int choosenIdx = rand() % nPoints;
			SiblingPointsPrefixMapEntry choosenEntry = *( --prefixMap.upper_bound( choosenIdx ) );
			selectedPoints[ i ] = choosenEntry.second.getContents()[ choosenIdx - choosenEntry.first ];
		}
		
		return selectedPoints;
	}
	
	template< typename Morton >
	inline string HierarchyCreator< Morton >::nodeListToString( const NodeList& list, const OctreeDim& lvlDim )
	{
		stringstream ss;
		ss << "list size: " << list.size() <<endl;
		for( const Node& node : list )
		{
			ss << lvlDim.calcMorton( node ).getPathToRoot( true );
		}
		
		return ss.str();
	}
	
	template< typename Morton >
	inline string HierarchyCreator< Morton >::workListToString( const WorkList& list, const OctreeDim& lvlDim )
	{
		stringstream ss; ss << "Size: " << list.size() << endl;
		
		for( const NodeList& nodeList : list )
		{
			ss << nodeListToString( nodeList, lvlDim ) << endl;
		}
		
		return ss.str();
	}
}

#undef NODE_COLAPSE

#undef LEAF_CREATION_DEBUG
#undef INNER_CREATION_DEBUG
#undef NODE_PROCESSING_DEBUG
#undef NODE_LIST_MERGE_DEBUG
#undef PARENT_DEBUG

#endif