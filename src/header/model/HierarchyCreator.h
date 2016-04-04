#ifndef HIERARCHY_CREATOR_H
#define HIERARCHY_CREATOR_H

#include <mutex>
#include <fstream>
#include <condition_variable>
#include <future>
#include "ManagedAllocator.h"
#include "O1OctreeNode.h"
#include "OctreeDimensions.h"
#include "Front.h"
#include "SQLiteManager.h"

//#define HIERARCHY_STATS

using namespace util;

namespace model
{
	// TODO: If this algorithm is the best one, change MortonCode API to get rid of shared_ptr.
	/** Multithreaded massive octree hierarchy creator. */
	template< typename Morton, typename Point >
	class HierarchyCreator
	{
	public:
		using PointPtr = shared_ptr< Point >;
		using PointVector = vector< PointPtr, ManagedAllocator< PointPtr > >;
		using Node = O1OctreeNode< PointPtr >;
		using NodeArray = Array< Node >;
		
		using OctreeDim = OctreeDimensions< Morton, Point >;
		using Front = model::Front< Morton, Point >;
		using Reader = PlyPointReader< Point >;
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
		/** Creates the hierarchy.
		 * @return hierarchy's root node. The pointer ownership is caller's. */
		Node* create();
		
		void pushWork( NodeList&& workItem );
		
		NodeList popWork( const int lvl );
		
		size_t updatedWorkListSize( int lvl );
		
		/** Sets the parent pointer for all children of a given node. */
		void setParent( Node& node, const int threadIdx ) const;
		
		/** If needed, collapse (turn into leaf) the boundary nodes of a worklist. */
		void collapseBoundaries( NodeList& list, const OctreeDim& nextLvlDim ) const;
		
		/** If needed, removes the boundary duplicate node in previousProcessed, moving its children to nextProcessed.
		 * Boundary duplicates can occur if nodes from the same sibling group are processed in different threads. */
		void removeBoundaryDuplicate( NodeList& previousProcessed, NodeList& nextProcessed, const OctreeDim& nextLvlDim )
		const;
		
		/** Merge previousProcessed into nextProcessed if there is not enough work yet to form a WorkList or push it to
		 * the next level WorkList otherwise. Repetitions are checked while linking lists, since it can occur when the
		 * lists have nodes from the same sibling group. */
		void mergeOrPushWork( NodeList& previousProcessed, NodeList& nextProcessed, OctreeDim& nextLvlDim );
		
		/** Creates a node from its solo child node. */
		Node createNodeFromSingleChild( Node&& child, bool isLeaf, const int threadIdx ) const;
		
		/** Creates an inner Node, given a children array and the number of meaningfull entries in the array. */
		Node createInnerNode( NodeArray&& inChildren, uint nChildren, const int threadIdx ) const;
		
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
		
		// Debug
		mutex m_logMutex;
// 		ofstream m_log;
		//
		
		ulong m_memoryLimit;
		
		ulong m_expectedLoadPerThread;
		
		Front& m_front;
		
		int m_nThreads;
	};
	
	template< typename Morton, typename Point >
	HierarchyCreator< Morton, Point >
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
		
// 		string dbFilename = m_plyFilename.substr( 0, m_plyFilename.find_last_of( "." ) ).append( ".db" );
// 		
// 		// Debug
// 		{
// 			cout << "Mem soft limit: " << m_memoryLimit << endl << endl;
// 		}
// 		
// 		m_dbs[ 0 ].init( dbFilename );
// 		for( int i = 1; i < m_nThreads; ++i )
// 		{
// 			m_dbs[ i ].init( dbFilename, false );
// 		}
		
		omp_set_num_threads( m_nThreads );
		
		// Debug
// 			{
// 				m_log.open( m_plyFilename.substr( 0, m_plyFilename.find_last_of( "." ) ).append( ".log" ) );
// 			}
	}
	
	template< typename Morton, typename Point >
	future< typename HierarchyCreator< Morton, Point >::Node* > HierarchyCreator< Morton, Point >::createAsync()
	{
		packaged_task< Node*() > task( [ & ]{ return create(); } );
		auto future = task.get_future();
		thread t( std::move( task ) );
		t.detach();
		
		return future;
	}
	
	template< typename Morton, typename Point >
	typename HierarchyCreator< Morton, Point >::Node* HierarchyCreator< Morton, Point >::create()
	{
		// Debug
		{
			cout << "MEMORY BEFORE CREATING: " << AllocStatistics::totalAllocated() << endl << endl;
		}
		
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
				reader.read( Reader::SINGLE,
					[ & ]( const Point& p )
					{
						Morton code = leafLvlDimCpy.calcMorton( p );
						Morton parent = *code.traverseUp();
						
						if( parent != currentParent )
						{
							if( points.size() > 0 )
							{
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
							
										// Debug
// 										{
// 											lock_guard< mutex > lock( m_logMutex );
// 											cout << "Disk thread stopped" << endl << endl;
// 										}
										
										unique_lock< mutex > lock( releaseMutex );
										releaseFlag.wait( lock, [ & ] { return !isReleasing; } );
									}
								}
							}
							currentParent = parent;
						}
						
						points.push_back( makeManaged< Point >( p ) );
					}
				);
				
				nodeList.push_back( Node( std::move( points ), true ) );
				pushWork( std::move( nodeList ) );
				
				leafLvlLoaded = true;
				m_front.notifyLeafLvlLoaded();
				
				// Debug
				{
					lock_guard< mutex > lock( m_logMutex );
					cout << "===== Leaf lvl loaded =====" << endl << endl;
				}
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
			
			if( isReleasing )
			{
				turnReleaseOff( releaseMutex, isReleasing, releaseFlag, diskThreadMutex, isDiskThreadStopped );
			}
			
			// BEGIN HIERARCHY CONSTRUCTION LOOP.
			while( lvl )
			{
				// Debug
// 				{
// 					lock_guard< mutex > lock( m_logMutex );
// 					cout << "======== Starting lvl " << lvl << ". Mem: " << AllocStatistics::totalAllocated()
// 						 << " ========" << endl << endl;
// 				}
				
				OctreeDim nextLvlDim( m_octreeDim, m_octreeDim.m_nodeLvl - 1 );
				
				bool increaseLvlFlag = false;
				
				size_t workListSize = updatedWorkListSize( lvl );
				
				// BEGIN HIERARCHY LEVEL LOOP.
				while( workListSize > 0 && !increaseLvlFlag )
				{
					// Debug
// 					{
// 						lock_guard< mutex > lock( m_logMutex );
// 						cout << "iter start" << endl << endl;
// 					}
					
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
					
					// Debug
// 					{
// 						lock_guard< mutex > lock( m_logMutex );
// 						cout << "Dispatched: " << dispatchedThreads << endl << endl;
// 					}
					
					IterArray iterInput( dispatchedThreads );
					
					for( int i = dispatchedThreads - 1; i > -1; --i )
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
						
// 						if( isReleasing )
// 						{
// 							m_dbs[ threadIdx ].beginTransaction();
// 						}
						
						while( !input.empty() )
						{
							Node& node = input.front();
							Morton parentCode = *m_octreeDim.calcMorton( node ).traverseUp();
							
							NodeArray siblings( 8 );
							siblings[ 0 ] = std::move( node );
							input.pop_front();
							int nSiblings = 1;
							
							// Debug
// 							{
// 								lock_guard< mutex > lock( m_logMutex );
// 								cout << "T " << omp_get_thread_num() << " sib[ 0 ]: "
// 										<< m_octreeDim.calcMorton( siblings[ 0 ] ).getPathToRoot( true );
// 							}
							//
							
							while( !input.empty() && *m_octreeDim.calcMorton( input.front() ).traverseUp() == parentCode )
							{
								siblings[ nSiblings ] = std::move( input.front() );
								++nSiblings;
								input.pop_front();
								
								// Debug
// 								{
// 									lock_guard< mutex > lock( m_logMutex );
// 									cout << "T " << omp_get_thread_num() << " sibl[ " << nSiblings - 1 << " ]: "
// 										<< m_octreeDim.calcMorton( siblings[ nSiblings - 1 ] ).getPathToRoot( true );
// 								}
							}
							
							bool isLastSiblingGroup = input.empty();
							
							if( workListSize - dispatchedThreads == 0 && !isLastPass && threadIdx == 0 && isLastSiblingGroup )
							{
								// Debug
// 								{
// 									lock_guard< mutex > lock( m_logMutex );
// 									cout << "Pushing sib group back." << endl << endl;
// 								}
								
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
								#ifdef HIERARCHY_STATS
									m_processedNodes += nSiblings;
								#endif
								
								if( isLastSiblingGroup )
								{
									isBoundarySiblingGroup = true;
								}
								
								if( nSiblings == 1 && siblings[ 0 ].isLeaf() && !isBoundarySiblingGroup )
								{
									// Debug
// 									{
// 										lock_guard< mutex > lock( m_logMutex );
// 										cout << "T " << omp_get_thread_num() << " collapse." << endl << endl;
// 									}
									
									output.push_back( createNodeFromSingleChild( std::move( siblings[ 0 ] ), true,
																				 threadIdx ) );
								}
								else
								{
									// Debug
// 									{
// 										lock_guard< mutex > lock( m_logMutex );
// 										cout << "T " << omp_get_thread_num() << " LOD." << endl << endl;
// 									}
									
									// LOD
									Node inner = createInnerNode( std::move( siblings ), nSiblings, threadIdx );
									
// 									if( isReleasing && !isBoundarySiblingGroup )
// 									{
// 										// Debug
// // 										{
// // 											lock_guard< mutex > lock( m_logMutex );
// // 											cout << "T " << omp_get_thread_num() << " release." << endl << endl;
// // 										}
// 										
// 										releaseSiblings( inner.child(), threadIdx, m_octreeDim );
// 									}
									
									output.push_back( std::move( inner ) );
									isBoundarySiblingGroup = false;
								}
							}
						}
						
// 						if( isReleasing )
// 						{
// 							m_dbs[ threadIdx ].endTransaction();
// 							
// 							// Debug 
// // 							{
// // 								Morton expectedMorton; expectedMorton.build( 0x20b2bffb54f9UL );
// // 								if( m_dbs[ threadIdx ].getNode( expectedMorton ).first == true )
// // 								{
// // 									if( foundMarked == false )
// // 									{
// // 										foundMarked = true;
// // 									}
// // 								}
// // 								else
// // 								{
// // 									if( foundMarked == true )
// // 									{
// // 										throw logic_error( "Node 0x20b2bffb54f9UL disappered from DB." );
// // 									}
// // 								}
// // 							}
// 						}
					}
					// END PARALLEL WORKLIST PROCESSING.
					
					m_front.notifyInsertionEnd();
					
					// BEGIN LOAD BALANCE.
					// Debug
// 					{
// 						cout << "Before work merge:" << endl;
// 						for( int i = 0; i < iterOutput.size(); ++i )
// 						{
// 							cout << "output " << i << endl
// 									<< nodeListToString( iterOutput[ i ], nextLvlDim );
// 						}
// 						cout << endl;
// 					}
					
					WorkList& nextLvlWorkList = m_lvlWorkLists[ lvl - 1 ];
					
					if( !nextLvlWorkList.empty() && !iterOutput.empty() && !iterOutput[ dispatchedThreads - 1 ].empty() )
					{
						NodeList nextLvlBack = std::move( nextLvlWorkList.back() );
						nextLvlWorkList.pop_back();
						
						// Debug
// 						{
// 							lock_guard< mutex > lock( m_logMutex );
// 							cout << "Merging nextLvl back and output " << dispatchedThreads - 1 << endl
// 								 << "Next lvl back size: " << nextLvlBack.size() << "output size: "
// 								 << iterOutput[ dispatchedThreads - 1 ].size() << endl << endl;
// 						}
						
						mergeOrPushWork( nextLvlBack, iterOutput[ dispatchedThreads - 1 ], nextLvlDim );
					}
					
					for( int i = dispatchedThreads - 1; i > 0; --i )
					{
						// Debug
// 						{
// 							cout << "Merging output " << i << " and " << i - 1 << endl << endl;
// 						}
						
						mergeOrPushWork( iterOutput[ i ], iterOutput[ i - 1 ], nextLvlDim );
					}
					
					// Debug
// 					{
// 						cout << "Pushing last list to next lvl." << endl << endl;
// 					}
					
					// The last thread NodeList is not collapsed, since the last node can be in a sibling group not
					// entirely processed in this iteration.
					if( !iterOutput.empty() && !iterOutput[ 0 ].empty() )
					{
						if( !nextLvlWorkList.empty() )
						{
							removeBoundaryDuplicate( nextLvlWorkList.back(), iterOutput[ 0 ], nextLvlDim );
						}
						nextLvlWorkList.push_back( std::move( iterOutput[ 0 ] ) );
					}
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
						// The last NodeList is not collapsed yet.
						collapseBoundaries( m_lvlWorkLists[ lvl - 1 ].back(), nextLvlDim );
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
		
		// Fixing root's child parent pointers.
		NodeArray& rootChild = root->child();
		for( int i = 0; i < rootChild.size(); ++i )
		{
			rootChild[ i ].setParent( root );
		}
		
		return root;
	}
	
	template< typename Morton, typename Point >
	inline void HierarchyCreator< Morton, Point >::pushWork( NodeList&& workItem )
	{
		lock_guard< mutex > lock( m_listMutex );
		
		// Debug
// 			if( workItem.size() < m_expectedLoadPerThread )
// 			{
// 				lock_guard< mutex > lock( m_logMutex );
// 				cout << "Pushing work: " << workItem.size() << endl << endl;
// 			}
		
		m_lvlWorkLists[ m_leafLvlDim.m_nodeLvl ].push_back( std::move( workItem ) );
	}
	
	template< typename Morton, typename Point >
	inline typename HierarchyCreator< Morton, Point >::NodeList HierarchyCreator< Morton, Point >
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
	
	template< typename Morton, typename Point >
	inline size_t HierarchyCreator< Morton, Point >::updatedWorkListSize( int lvl )
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
	template< typename Morton, typename Point >
	inline void HierarchyCreator< Morton, Point >::setParent( Node& node, const int threadIdx ) const
	{

		// Debug
// 			{
// 				Morton morton; morton.build( 0x8 );
// 				if( m_octreeDim.calcMorton( node ) == morton )
// 				{
// 					cout << "Setting children parents of " << m_octreeDim.calcMorton( node ).getPathToRoot( true ) << endl;
// 				}
// 			}
		
		OctreeDim childDim( m_octreeDim, m_octreeDim.m_nodeLvl + 1 );
		NodeArray& children = node.child();
		for( int i = 0; i < children.size(); ++i )
		{
			// Debug
// 				{
// 					Morton morton; morton.build( 0x8 );
// 					if( m_octreeDim.calcMorton( node ) == morton )
// 					{
// 						OctreeDim childDim( m_octreeDim, m_octreeDim.m_nodeLvl + 1 );
// 						cout << "Setting parent of " << childDim.calcMorton( children[ i ] ).getPathToRoot( true )
// 							<< ": " << m_octreeDim.calcMorton( node ).getPathToRoot( true )
// 							<< "addr: " << &node << endl << endl;
// 					}
// 				}
			
			Node& child = children[ i ];
			child.setParent( &node );
			
			if( child.isLeaf() )
			{
				m_front.insertIntoFront( child, childDim.calcMorton( child ), threadIdx );
			}
		}
	}
		
	/** If needed, collapse (turn into leaf) the boundary nodes of a worklist. */
	template< typename Morton, typename Point >
	inline void HierarchyCreator< Morton, Point >::collapseBoundaries( NodeList& list, const OctreeDim& nextLvlDim )
	const
	{
		if( !list.empty() )
		{
			Node& firstNode = list.front();
			NodeArray& firstNodeChild = firstNode.child();
			
			// Debug
// 				{
// 					cout << "Checking for collapse list with size " << list.size() << ":" << endl;
// 					cout << nextLvlDim.calcMorton( firstNode ).getPathToRoot( true ) << endl;
// 				}
			
			if( firstNodeChild.size() == 1 && firstNodeChild[ 0 ].isLeaf() )
			{
				// Debug
// 					{
// 						cout << "Collapsing." << endl << endl;
// 					}
				//
				
				firstNode.turnLeaf();
			}
			
			Node& lastNode = list.back();
			NodeArray& lastNodeChild = lastNode.child();
			
			// Debug
// 				{
// 					cout << "Checking for collapse: " << nextLvlDim.calcMorton( lastNode ).getPathToRoot( true ) << endl;
// 				}
			
			if( lastNodeChild.size() == 1 && lastNodeChild[ 0 ].isLeaf() )
			{
				// Debug
// 					{
// 						cout << "Turning into leaf: " << nextLvlDim.calcMorton( lastNode ).getPathToRoot( true )
// 								<< endl;
// 					}
				//
				
				lastNode.turnLeaf();
			}
		}
	}
		
	// TODO: This method is wrong. The duplicate removal should also choose again the node points, since the children
	// changed.
	/** If needed, removes the boundary duplicate node in previousProcessed, moving its children to nextProcessed.
		* Boundary duplicates can occur if nodes from the same sibling group are processed in different threads. */
	template< typename Morton, typename Point >
	inline void HierarchyCreator< Morton, Point >
	::removeBoundaryDuplicate( NodeList& previousProcessed, NodeList& nextProcessed, const OctreeDim& nextLvlDim ) const
	{
		if( !previousProcessed.empty() && !nextProcessed.empty() )
		{
			Node& prevLastNode = previousProcessed.back();
			Node& nextFirstNode = nextProcessed.front();
			
			if( nextLvlDim.calcMorton( prevLastNode ) == nextLvlDim.calcMorton( nextFirstNode ) )
			{
				// Nodes from same sibling groups were in different threads
				// TODO: the merge operation needs to select again the points in nextFirstNode.
				
				// Debug
// 				{
// 					cout << "Duplicate found: " << nextLvlDim.calcMorton( prevLastNode ).getPathToRoot( true ) << endl << endl;
// 				}
				
				NodeArray& prevLastNodeChild = prevLastNode.child();
				NodeArray& nextFirstNodeChild = nextFirstNode.child();
				
				NodeArray mergedChild( prevLastNodeChild.size() + nextFirstNodeChild.size() );
				
				for( int i = 0; i < prevLastNodeChild.size(); ++i )
				{
					mergedChild[ i ] = std::move( prevLastNodeChild[ i ] );
					// Parent pointer needs to be fixed.
					setParent( mergedChild[ i ], 0 );
				}
				for( int i = 0; i < nextFirstNodeChild.size(); ++i )
				{
					int idx = prevLastNodeChild.size() + i;
					mergedChild[ idx ] = std::move( nextFirstNodeChild[ i ] );
					// Parent pointer needs to be fixed.
					setParent( mergedChild[ idx ], 0 );
				}
				
				nextFirstNode.setChildren( std::move( mergedChild ) );
				
				// Debug
// 				{
// 					cout << "Merged child: " << endl;
// 					NodeArray& newChild = nextFirstNode.child();
// 					for( int i = 0; i < newChild.size(); ++i )
// 					{
// 						cout << m_octreeDim.calcMorton( newChild[ i ] ).getPathToRoot( true );
// 					}
// 					cout << endl;
// 				}
				
				previousProcessed.pop_back();
			}
			
			collapseBoundaries( previousProcessed, nextLvlDim );
			
			// If needed, collapse the first node of nextProcessed
			NodeArray& nextFirstNodeChild = nextFirstNode.child();
			if( nextFirstNodeChild.size() == 1 && nextFirstNodeChild[ 0 ].isLeaf() )
			{
				nextFirstNode.turnLeaf();
			}
		}
	}
		
	/** Merge previousProcessed into nextProcessed if there is not enough work yet to form a WorkList or push it to
		* the next level WorkList otherwise. Repetitions are checked while linking lists, since it can occur when the
		* lists have nodes from the same sibling group. */
	template< typename Morton, typename Point >
	inline void HierarchyCreator< Morton, Point >
	::mergeOrPushWork( NodeList& previousProcessed, NodeList& nextProcessed, OctreeDim& nextLvlDim )
	{
		// Debug
// 			{
// 				cout << "mergeOrPushWork begin" << endl << "prev:" << endl << nodeListToString( previousProcessed, nextLvlDim )
// 					 << endl << "next:" << endl << nodeListToString( nextProcessed, nextLvlDim ) << endl
// 					 << "nextLvlWorkList:" << endl << workListToString( m_lvlWorkLists[ nextLvlDim.m_nodeLvl ], nextLvlDim )
// 					 << "nextLvlWorkList end" << endl << endl;
// 			}
		
		removeBoundaryDuplicate( previousProcessed, nextProcessed, nextLvlDim );
	
		if( previousProcessed.size() < m_expectedLoadPerThread )
		{
			// Debug
// 				{
// 					cout << "Moving prev to next" << endl << endl;
// 				}
			
			nextProcessed.splice( nextProcessed.begin(), previousProcessed );
		}
		else
		{
			// Debug
// 				{
// 					cout << "Pushing next to global worklist." << endl << endl;
// 				}
			
			WorkList& workList = m_lvlWorkLists[ nextLvlDim.m_nodeLvl ];
			
			if( !workList.empty() )
			{
				removeBoundaryDuplicate( workList.back(), previousProcessed, nextLvlDim );
			}
			
			workList.push_back( std::move( previousProcessed ) );
		}
		
		// Debug
// 			{
// 				cout << "mergeOrPushWork end" << endl << "prev:" << endl << nodeListToString( previousProcessed, nextLvlDim ) << endl
// 					<< "next:" << endl << nodeListToString( nextProcessed, nextLvlDim ) << endl
// 					<< "nextLvlWorkList:" << endl << workListToString( m_lvlWorkLists[ nextLvlDim.m_nodeLvl ], nextLvlDim ) << "nextLvlWorkList end" << endl << endl;
// 			}
	}
	
	template< typename Morton, typename Point >
	inline bool HierarchyCreator< Morton, Point >::checkAllWorkFinished()
	{
		lock_guard< mutex > lock( m_listMutex );
		
		for( int i = 1; i < m_lvlWorkLists.size(); ++i )
		{
			if( !m_lvlWorkLists[ i ].empty() )
			{
				// Debug
// 					{
// 						cout << "Works remaining" << endl << endl;
// 					}
				
				return false;
			}
		}
		
		// Debug
// 			{
// 				cout << "All work finished" << endl << endl;
// 			}
		
		return true;
	}
	
	template< typename Morton, typename Point >
	inline void HierarchyCreator< Morton, Point >::turnReleaseOn( mutex& releaseMutex, bool& isReleasing )
	{
		
		// Debug
// 		{
// 			cout << "===== RELEASE ON: Mem " << AllocStatistics::totalAllocated() << " =====" << endl
// 					<< endl;
// 		}
		
		{
			lock_guard< mutex > lock( releaseMutex );
			isReleasing = true;
		}
		
		m_front.turnReleaseOn();
	}
		
	template< typename Morton, typename Point >
	inline void HierarchyCreator< Morton, Point >
	::turnReleaseOff( mutex& releaseMutex, bool& isReleasing, condition_variable& releaseFlag, mutex& diskThreadMutex,
						bool& isDiskThreadStopped )
	{
		// Debug
// 		{
// 			lock_guard< mutex > lock( m_logMutex );
// 			cout << "===== RELEASE OFF. MEMORY: " << AllocStatistics::totalAllocated() << "=====" << endl << endl;
// 		}
		
		{
			lock_guard< mutex > lock( releaseMutex );
			isReleasing = false;
		}
		releaseFlag.notify_one();
		
		m_front.turnReleaseOff();
		
		// Debug
// 			{
// 				lock_guard< mutex > lock( m_logMutex );
// 				cout << "Disk thread resumed" << endl << endl;
// 			}
		//
		
		{
			lock_guard< mutex > lock( diskThreadMutex );
			isDiskThreadStopped = false;
		}
	}
	
// 	template< typename Morton, typename Point >
// 	inline void HierarchyCreator< Morton, Point >
// 	::releaseSiblings( NodeArray& siblings, const int threadIdx, const OctreeDim& dim )
// 	{
// 		Sql& sql = m_dbs[ threadIdx ];
// 
// 		// Debug
// // 		{
// // 			lock_guard< mutex > lock( m_logMutex );
// // 			cout << "Thread " << threadIdx << ": rel sib size " << siblings.size() << endl << endl;
// // 		}
// 		
// 		for( int i = 0; i < siblings.size(); ++i )
// 		{
// 			Node& sibling = siblings[ i ];
// 			NodeArray& children = sibling.child();
// 			if( !children.empty() )
// 			{
// 				releaseSiblings( children, threadIdx, OctreeDim( dim, dim.m_nodeLvl + 1 ) );
// 			}
// 			
// 			Morton siblingMorton = dim.calcMorton( sibling );
// 			// Debug
// // 			{
// // 				Morton expectedParent; expectedParent.build( 0x41657ff6a9fUL );
// // 				Morton expectedA = *expectedParent.getFirstChild();
// // 				Morton expectedB = *expectedParent.getLastChild();
// // 				
// // 				if( expectedA <= siblingMorton && siblingMorton <= expectedB )
// // 				{
// // 					cout << "2DB: " << hex << siblingMorton.getPathToRoot( true ) << endl;
// // 				}
// // 			}
// 			
// 			// Persisting node
// 			sql.insertNode( siblingMorton, sibling );
// 		}
// 		
// 		siblings.clear();
// 	}
	
	template< typename Morton, typename Point >
	inline typename HierarchyCreator< Morton, Point >::Node HierarchyCreator< Morton, Point >
	::createNodeFromSingleChild( Node&& child, bool isLeaf, const int threadIdx ) const
	{
		// Setup a placeholder in front if the node is in the leaf level.
		Morton childMorton = m_octreeDim.calcMorton( child );
		if( childMorton.getLevel() == m_leafLvlDim.m_nodeLvl )
		{
			m_front.insertPlaceholder( childMorton, threadIdx );
		}
		
		const Array< PointPtr >& childPoints = child.getContents();
		
		int numSamplePoints = std::max( 1., childPoints.size() * 0.125 );
		Array< PointPtr > selectedPoints( numSamplePoints );
		
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
			if( !finalChild.isLeaf() )
			{
				setParent( finalChild, threadIdx );
			}
		}
		
		return node;
	}
	
	// TODO: Probably there is no need for the first for loop...
	template< typename Morton, typename Point >
	inline typename HierarchyCreator< Morton, Point >::Node HierarchyCreator< Morton, Point >
	::createInnerNode( NodeArray&& inChildren, uint nChildren, const int threadIdx ) const
	{
		using Map = map< int, Node&, less< int >, ManagedAllocator< pair< const int, Node& > > >;
		using MapEntry = typename Map::value_type;
		
		if( nChildren == 1 )
		{
			return createNodeFromSingleChild( std::move( inChildren[ 0 ] ), false, threadIdx );
		}
		else
		{
			NodeArray children( nChildren );
			
			// Verify if placeholders are necessary.
			bool frontPlaceholdersOn = ( m_octreeDim.calcMorton( inChildren[ 0 ] ).getLevel() == m_leafLvlDim.m_nodeLvl );
			
			for( int i = 0; i < children.size(); ++i )
			{
				children[ i ] = std::move( inChildren[ i ] );
				
				if( frontPlaceholdersOn )
				{
					m_front.insertPlaceholder( m_octreeDim.calcMorton( children[ i ] ), threadIdx );
				}
				
				// Debug
// 				if( i > 0 )
// 				{
// 					Morton prevMorton = m_octreeDim.calcMorton( children[ i - 1 ] );
// 					Morton currentMorton = m_octreeDim.calcMorton( children[ i ] );
// 					if( currentMorton <= prevMorton )
// 					{
// 						stringstream ss;
// 						ss 	<< "Current: " << currentMorton.getPathToRoot( true ) << " <= "
// 							<< prevMorton.getPathToRoot( true ) << endl;
// 						throw logic_error( ss.str() );
// 					}
// 				}
				//
			}
			
			int nPoints = 0;
			Map prefixMap;
			for( int i = 0; i < children.size(); ++i )
			{
				Node& child = children[ i ];
				
				prefixMap.insert( prefixMap.end(), MapEntry( nPoints, child ) );
				nPoints += child.getContents().size();
				
				// Set parental relationship of children.
				if( !child.isLeaf() )
				{
					setParent( child, threadIdx );
				}
			}
			
			// LoD has 1/8 of children points.
			int numSamplePoints = std::max( 1., nPoints * 0.125 );
			Array< PointPtr > selectedPoints( numSamplePoints );
			
			for( int i = 0; i < numSamplePoints; ++i )
			{
				int choosenIdx = rand() % nPoints;
				MapEntry choosenEntry = *( --prefixMap.upper_bound( choosenIdx ) );
				selectedPoints[ i ] = choosenEntry.second.getContents()[ choosenIdx - choosenEntry.first ];
			}
			
			Node node( std::move( selectedPoints ), false );
			node.setChildren( std::move( children ) );
			
			// Debug
// 			{
// 				Morton expectedMorton; expectedMorton.build( 0x8 );
// 				NodeArray& children = node.child();
// 				for( int i; i < children.size(); ++i  )
// 				{
// 					Morton nodeMorton = m_octreeDim.calcMorton( children[ i ] );
// 					if( nodeMorton == expectedMorton )
// 					{
// 						cout << "FINAL NODE ADDR: " << &children[ i ] << endl << endl;
// 						break;
// 					}
// 				}
// 			}
			
			return node;
		}
	}
	
	template< typename Morton, typename Point >
	inline string HierarchyCreator< Morton, Point >::nodeListToString( const NodeList& list, const OctreeDim& lvlDim )
	{
		stringstream ss;
		ss << "list size: " << list.size() <<endl;
		for( const Node& node : list )
		{
			ss << lvlDim.calcMorton( node ).getPathToRoot( true );
		}
		
		return ss.str();
	}
	
	template< typename Morton, typename Point >
	inline string HierarchyCreator< Morton, Point >::workListToString( const WorkList& list, const OctreeDim& lvlDim )
	{
		stringstream ss;
		
		for( const NodeList& nodeList : list )
		{
			ss << nodeListToString( nodeList, lvlDim ) << endl;
		}
		
		return ss.str();
	}
}

#endif