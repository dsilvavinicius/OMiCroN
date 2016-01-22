#ifndef HIERARCHY_CREATOR_H
#define HIERARCHY_CREATOR_H

#include <mutex>
#include <condition_variable>
#include "ManagedAllocator.h"
#include "O1OctreeNode.h"
#include "OctreeDimensions.h"
#include "SQLiteManager.h"

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
		using Reader = PlyPointReader< Point >;
		using Sql = SQLiteManager< Point, Morton, Node >;
		
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
		HierarchyCreator( const string& sortedPlyFilename, const OctreeDim& dim, ulong expectedLoadPerThread,
						  const ulong memoryLimit )
		: m_plyFilename( sortedPlyFilename ), 
		m_lvlWorkLists( dim.m_nodeLvl + 1 ),
		m_leafLvlDim( dim ),
		m_expectedLoadPerThread( expectedLoadPerThread ),
		m_dbs( M_N_THREADS ),
		m_memoryLimit( memoryLimit )
		{
			srand( 1 );
			
			string dbFilename = m_plyFilename.substr( 0, m_plyFilename.find_last_of( "." ) ).append( ".db" );
			
			// Debug
			{
				cout << "Mem soft limit: " << m_memoryLimit << endl << endl;
			}
			
			for( int i = 0; i < M_N_THREADS; ++i )
			{
				m_dbs[ i ].init( dbFilename );
			}
		}
		
		/** Creates the hierarchy.
		 * @return hierarchy's root node. The pointer ownership is caller's. */
		Node* create();
		
	private:
		void pushWork( NodeList&& workItem )
		{
			lock_guard< mutex > lock( m_listMutex );
			
			// Debug
// 			{
// 				cout << "Pushing work." << endl << endl;
// 			}
			
			m_lvlWorkLists[ m_leafLvlDim.m_nodeLvl ].push_back( std::move( workItem ) );
		}
		
		NodeList popWork( const int lvl )
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
		
		size_t updatedWorkListSize( int lvl )
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
		void setParent( Node& node ) const
		{
			// Debug
// 			{
// 				cout << "Setting children parents of " << m_octreeDim.calcMorton( node ).getPathToRoot( true ) << endl;
// 			}
			
			NodeArray& children = node.child();
			for( int i = 0; i < children.size(); ++i )
			{
				// Debug
// 				{
// 					OctreeDim childDim( m_octreeDim, m_octreeDim.m_nodeLvl + 1 );
// 					cout << "Setting parent of " << childDim.calcMorton( children[ i ] ).getPathToRoot( true )
// 						<< ": " << m_octreeDim.calcMorton( node ).getPathToRoot( true )
// 						<< "addr: " << &node << endl << endl;
// 				}
				
				children[ i ].setParent( &node );
			}
		}
		
		/** If needed, collapse (turn into leaf) the boundary nodes of a worklist. */
		void collapseBoundaries( NodeList& list, const OctreeDim& nextLvlDim ) const
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
		
		/** If needed, removes the boundary duplicate node in previousProcessed, moving its children to nextProcessed.
		 * Boundary duplicates can occur if nodes from the same sibling group are processed in different threads. */
		void removeBoundaryDuplicate( NodeList& previousProcessed, NodeList& nextProcessed, const OctreeDim& nextLvlDim )
		const
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
						setParent( mergedChild[ i ] );
					}
					for( int i = 0; i < nextFirstNodeChild.size(); ++i )
					{
						int idx = prevLastNodeChild.size() + i;
						mergedChild[ idx ] = std::move( nextFirstNodeChild[ i ] );
						// Parent pointer needs to be fixed.
						setParent( mergedChild[ idx ] );
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
		void mergeOrPushWork( NodeList& previousProcessed, NodeList& nextProcessed, OctreeDim& nextLvlDim )
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
		
		/** Creates a node from its solo child node. */
		Node createNodeFromSingleChild( Node&& child, bool isLeaf ) const;
		
		/** Creates an inner Node, given a children array and the number of meaningfull entries in the array. */
		Node createInnerNode( NodeArray&& inChildren, uint nChildren ) const;
		
		/** Checks if all work is finished in all lvls. */
		bool checkAllWorkFinished()
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
		
		/** Releases and persists a given sibling group.
		 * @param siblings is the sibling group to be released and persisted if it is the case.
		 * @param threadIdx is the index of the executing thread.
		 * @param dim is the dimensions of the octree for the sibling lvl. */
		void releaseSiblings( NodeArray& siblings, const int threadIdx, const OctreeDim& dim );
		
		string nodeListToString( const NodeList& list, const OctreeDim& lvlDim )
		{
			stringstream ss;
			ss << "list size: " << list.size() <<endl;
			for( const Node& node : list )
			{
				ss << lvlDim.calcMorton( node ).getPathToRoot( true );
			}
			
			return ss.str();
		}
		
		string workListToString( const WorkList& list, const OctreeDim& lvlDim )
		{
			stringstream ss;
			
			for( const NodeList& nodeList : list )
			{
				ss << nodeListToString( nodeList, lvlDim ) << endl;
			}
			
			return ss.str();
		}
		
		/** Thread[ i ] uses database connection m_sql[ i ]. */
		Array< Sql > m_dbs;
		
		/** Worklists for every level in the hierarchy. m_lvlWorkLists[ i ] corresponds to lvl i. */
		Array< WorkList > m_lvlWorkLists;
		
		/** Current lvl octree dimensions. */
		OctreeDim m_octreeDim;
		
		/** Octree dimensions of the leaf level in this octree. */
		OctreeDim m_leafLvlDim;
		
		string m_plyFilename;
		
		// TODO: m_listMutex should be replaced by a per-lvl mutex.
		/** Mutex for the work list. */
		mutex m_listMutex;
		
		// Debug
		mutex m_logMutex;
		//
		
		ulong m_memoryLimit;
		
		ulong m_expectedLoadPerThread;
		
		static constexpr int M_N_THREADS = 8;
	};
	
	template< typename Morton, typename Point >
	typename HierarchyCreator< Morton, Point >::Node* HierarchyCreator< Morton, Point >::create()
	{
		// Debug
		{
			cout << "MEMORY BEFORE CREATING: " << AllocStatistics::totalAllocated() << endl << endl;
		}
		
		// SHARED. The disk access thread sets this true when it finishes reading all points in the sorted file.
		bool leafLvlLoaded = false;
		
		mutex releaseMutex;
		bool isReleasing = false;
		bool isDiskThreadStopped = false;
		
		// SHARED. Indicates when a node release is ocurring.
		condition_variable releaseFlag;
		
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
						if( isReleasing )
						{
							isDiskThreadStopped = true;
							
							unique_lock< mutex > lock( releaseMutex );
							releaseFlag.wait( lock, [ & ]{ return !isReleasing; } );
							
							isDiskThreadStopped = false;
						}
						else 
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
									}
								}
								currentParent = parent;
							}
						}
						
						points.push_back( makeManaged< Point >( p ) );
					}
				);
				
				nodeList.push_back( Node( std::move( points ), true ) );
				pushWork( std::move( nodeList ) );
				
				leafLvlLoaded = true;
				
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
			
// 			if( isReleasing )
// 			{
// 				// Debug
// 				{
// 					lock_guard< mutex > lock( m_logMutex );
// 					cout << "RELEASE OFF. MEMORY: " << AllocStatistics::totalAllocated() << endl << endl;
// 				}
// 				
// 				for( int i = 0; i < M_N_THREADS; ++i )
// 				{
// 					m_dbs[ i ].endTransaction();
// 				}
// 				isReleasing = false;
// 			}
			
			//Debug
// 			{
// 				lock_guard< mutex > lock( m_logMutex );
// 				cout << "===== NEW ALGORITHM PASS =====" << endl;
// 				for( int i = 0; i < m_lvlWorkLists.size(); ++i )
// 				{
// 					cout << "Lvl " << i << " size: " << m_lvlWorkLists[ i ].size() << endl;
// 				}
// 				cout << endl;
// 			}
			
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
					if( workListSize > M_N_THREADS )
					{
						dispatchedThreads = M_N_THREADS;
					}
					else if( lvl != m_leafLvlDim.m_nodeLvl || isLastPass || isDiskThreadStopped )
					{
						dispatchedThreads = workListSize;
						increaseLvlFlag = true;
					}
					else
					{
						dispatchedThreads = workListSize - 1;
						increaseLvlFlag = true;
					}
					
					// Debug
// 					{
// 						lock_guard< mutex > lock( m_logMutex );
// 						cout << "Dispatched threads: " << dispatchedThreads << endl << endl;
// 					}
					
					IterArray iterInput( dispatchedThreads );
					
					for( int i = dispatchedThreads - 1; i > -1; --i )
					{
						iterInput[ i ] = popWork( lvl );
						
						// Debug
// 						{
// 							lock_guard< mutex > lock( m_logMutex );
// 							cout << "Pop size: " << iterInput[ i ].size() << endl << endl;
// 						}
					}
					
					IterArray iterOutput( dispatchedThreads );
					
					// BEGIN PARALLEL WORKLIST PROCESSING.
					#pragma omp parallel for
					for( int i = 0; i < dispatchedThreads; ++i )
					{
						NodeList& input = iterInput[ i ];
						NodeList& output = iterOutput[ i ];
						bool isBoundarySiblingGroup = true;
						
						if( isReleasing )
						{
							m_dbs[ i ].beginTransaction();
						}
						
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
// 								cout << "Thread " << omp_get_thread_num() << " sibling [ 0 ]: "
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
// 									cout << "Thread " << omp_get_thread_num() << " sibling [ " << nSiblings - 1 << " ]: "
// 										<< m_octreeDim.calcMorton( siblings[ nSiblings - 1 ] ).getPathToRoot( true );
// 								}
							}
							
							bool isLastSiblingGroup = input.empty();
							
							if( workListSize - dispatchedThreads == 0 && !isLastPass && i == 0 && isLastSiblingGroup )
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
									// Debug
// 									{
// 										lock_guard< mutex > lock( m_logMutex );
// 										cout << "Thread " << omp_get_thread_num() << " collapse." << endl << endl;
// 									}
									
									output.push_back( createNodeFromSingleChild( std::move( siblings[ 0 ] ), true ) );
								}
								else
								{
									// Debug
// 									{
// 										lock_guard< mutex > lock( m_logMutex );
// 										cout << "Thread " << omp_get_thread_num() << " LOD." << endl << endl;
// 									}
									
									// LOD
									Node inner = createInnerNode( std::move( siblings ), nSiblings );
									
									if( isReleasing && !isBoundarySiblingGroup )
									{
										// Debug
// 										{
// 											lock_guard< mutex > lock( m_logMutex );
// 											cout << "Thread " << omp_get_thread_num() << " will release. MEMORY BEFORE: "
// 												 << AllocStatistics::totalAllocated() << endl << endl;
// 										}
										
										releaseSiblings( inner.child(), omp_get_thread_num(), m_octreeDim );
										
										// Debug
// 										{
// 											cout << "MEMORY AFTER RELEASE: " << AllocStatistics::totalAllocated() << endl << endl;
// 										}
									}
									
									output.push_back( std::move( inner ) );
									isBoundarySiblingGroup = false;
								}
							}
						}
						
						if( isReleasing )
						{
							m_dbs[ i ].endTransaction();
						}
					}
					// END PARALLEL WORKLIST PROCESSING.
					
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
							//Debug
							{
								cout << "===== RELEASE OFF: Mem " << AllocStatistics::totalAllocated() << " =====" << endl << endl;
							}
							
// 							for( int i = 0; i < M_N_THREADS; ++i )
// 							{
// 								m_dbs[ i ].endTransaction();
// 							}
							
							isReleasing = false;
							releaseFlag.notify_one();
						}
					}
					else if( AllocStatistics::totalAllocated() > m_memoryLimit )
					{
						// Check memory stress and release memory if necessary.
						
						// Debug
						{
							cout << "===== RELEASE ON: Mem " << AllocStatistics::totalAllocated() << " =====" << endl
								 << endl;
						}
						
// 						for( int i = 0; i < M_N_THREADS; ++i )
// 						{
// 							m_dbs[ i ].beginTransaction();
// 						}
						
						isReleasing = true;
					}
					// END NODE RELEASE MANAGEMENT.
					
					workListSize = updatedWorkListSize( lvl );
					
					size_t leafLvlWorkCount = ( lvl == m_leafLvlDim.m_nodeLvl ) ? workListSize :
						updatedWorkListSize( m_leafLvlDim.m_nodeLvl );
					
					if( !isLastPass && workListSize < M_N_THREADS &&
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
	inline void HierarchyCreator< Morton, Point >
	::releaseSiblings( NodeArray& siblings, const int threadIdx, const OctreeDim& dim )
	{
		Sql& sql = m_dbs[ threadIdx ];

		// Debug
// 		{
// 			cout << "Thread " << threadIdx << " n siblings: " << siblings.size() << endl << endl;
// 		}
		
		for( int i = 0; i < siblings.size(); ++i )
		{
			Node& sibling = siblings[ i ];
			NodeArray& children = sibling.child();
			if( !children.empty() )
			{
				releaseSiblings( children, threadIdx, OctreeDim( dim, dim.m_nodeLvl + 1 ) );
			}
			
			Morton siblingMorton = dim.calcMorton( sibling );
			// Debug
// 			{
// 				lock_guard< mutex > lock( m_logMutex );
// 				cout << "Thread " << threadIdx << ": persisting sibling " << i << ": "
// 					 << siblingMorton.getPathToRoot( true ) << endl << endl;
// 			}
			
			// Persisting node
			sql.insertNode( siblingMorton, sibling );
		}
		
		siblings.clear();
	}
	
	template< typename Morton, typename Point >
	inline typename HierarchyCreator< Morton, Point >::Node HierarchyCreator< Morton, Point >
	::createNodeFromSingleChild( Node&& child, bool isLeaf ) const
	{
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
				setParent( finalChild );
			}
		}
		
		return node;
	}
	
	template< typename Morton, typename Point >
	inline typename HierarchyCreator< Morton, Point >::Node HierarchyCreator< Morton, Point >
	::createInnerNode( NodeArray&& inChildren, uint nChildren ) const
	{
		using Map = map< int, Node&, less< int >, ManagedAllocator< pair< const int, Node& > > >;
		using MapEntry = typename Map::value_type;
		
		if( nChildren == 1 )
		{
			return createNodeFromSingleChild( std::move( inChildren[ 0 ] ), false );
		}
		else
		{
			NodeArray children( nChildren );
			
			for( int i = 0; i < children.size(); ++i )
			{
				children[ i ] = std::move( inChildren[ i ] );
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
					setParent( child );
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
			
			return node;
		}
	}
}

#endif