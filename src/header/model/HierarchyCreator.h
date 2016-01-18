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
		m_octreeDim( dim ),
		m_expectedLoadPerThread( expectedLoadPerThread ),
		m_dbs( M_N_THREADS ),
		m_memoryLimit( memoryLimit )
		{
			srand( 1 );
			
			string dbFilename = m_plyFilename.substr( 0, m_plyFilename.find_last_of( "." ) ).append( ".db" );
			
			// Debug
// 			{
// 				cout << "DB filename: " << dbFilename << endl << endl;
// 			}
			
			for( int i = 0; i < M_N_THREADS; ++i )
			{
				m_dbs[ i ].init( dbFilename );
			}
		}
		
		/** Creates the hierarchy.
		 * @return hierarchy's root node. The pointer ownership is caller's. */
		Node* create();
		
	private:
		// TODO: The lock can be avoided after the disk thread has finished its work.
		void pushWork( const int lvl, NodeList&& workItem )
		{
			lock_guard< mutex > lock( m_listMutex );
			
			// Debug
			{
				cout << "Pushing work. Size: " << workItem.size() << endl << endl;
			}
			
			m_lvlWorkLists[ lvl ].push_back( std::move( workItem ) );
		}
		
		// TODO: The lock can be avoided after the disk thread has finished its work.
		NodeList popWork( const int lvl )
		{
			lock_guard< mutex > lock( m_listMutex );
			NodeList nodeList = std::move( m_lvlWorkLists[ lvl ].front() );
			m_lvlWorkLists[ lvl ].pop_front();
			return nodeList;
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
			assert( !previousProcessed.empty() && "previousProcessed empty." );
			assert( !nextProcessed.empty() && "nextProcessed empty." );
			
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
		
		/** Merge previousProcessed into nextProcessed if there is not enough work yet to form a WorkList or push it to
		 * nextLvlWorkLists otherwise. Repetitions are checked while linking lists, since it can occur when the lists
		 * have nodes from the same sibling group. */
		void mergeOrPushWork( NodeList& previousProcessed, NodeList& nextProcessed, OctreeDim& nextLvlDim )
		{
			// Debug
// 			{
// 				cout << "mergeOrPushWork begin" << endl << "prev:" << endl << nodeListToString( previousProcessed, nextLvlDim ) << endl
// 					<< "next:" << endl << nodeListToString( nextProcessed, nextLvlDim ) << endl
// 					<< "nextLvlWorkList:" << endl << workListToString( m_nextLvlWorkList, nextLvlDim ) << "nextLvlWorkList end" << endl << endl;
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
// 					<< "nextLvlWorkList:" << endl << workListToString( m_nextLvlWorkList, nextLvlDim ) << "nextLvlWorkList end" << endl << endl;
// 			}
		}
		
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
					{
						cout << "Works remaining" << endl << endl;
					}
					
					return false;
				}
			}
			
			// Debug
			{
				cout << "All work finished" << endl << endl;
			}
			
			return true;
		}
		
		/** Releases and persists a given sibling group.
		 * @param siblings is the sibling group to be released and persisted if it is the case.
		 * @param threadIdx is the index of the executing thread.
		 * @param dim is the dimensions of the octree for the sibling lvl. */
		void releaseSiblings( NodeArray& siblings, const int threadIdx, const OctreeDim& dim );
		
		/** Releases nodes in order to ease memory stress. */
		void releaseNodes();
		
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
		
		string m_plyFilename;
		
		// TODO: m_listMutex can be replaced by a per-lvl mutex.
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
		// SHARED. Indicates when a node release is ocurring.
		condition_variable releaseFlag;
		
		OctreeDim leafLvlDim = m_octreeDim;
		
		// Thread that loads data from sorted file or database.
		thread diskAccessThread(
			[ & ]()
			{
				OctreeDim leafLvlDimCpy = leafLvlDim; // Just to make sure it will be in thread local mem.
				NodeList nodeList;
				PointVector points;
				
				Morton currentParent;
				Reader reader( m_plyFilename );
				reader.read( Reader::SINGLE,
					[ & ]( const Point& p )
					{
						if( isReleasing )
						{
							unique_lock< mutex > lock( releaseMutex );
								releaseFlag.wait( lock, [ & ]{ return !isReleasing; } );
						}
						else 
						{
							Morton code = leafLvlDimCpy.calcMorton( p );
							Morton parent = *code.traverseUp();
							
							if( parent != currentParent )
							{
								if( points.size() > 0 )
								{
									// Debug
									{
										lock_guard< mutex > lock( m_logMutex );
										cout << "Node work. points: " << points.size() << endl << endl;
									}
									
									nodeList.push_back( Node( std::move( points ), true ) );
									points = PointVector();
									
									if( nodeList.size() == m_expectedLoadPerThread )
									{
										pushWork( leafLvlDimCpy.m_nodeLvl, std::move( nodeList ) );
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
				pushWork( leafLvlDimCpy.m_nodeLvl, std::move( nodeList ) );
				
				leafLvlLoaded = true;
				
				// Debug
				{
					lock_guard< mutex > lock( m_logMutex );
					cout << "Leaf lvl loaded" << endl << endl;
				}
			}
		);
		diskAccessThread.detach();
		
		while( !leafLvlLoaded || !checkAllWorkFinished() )
		{
			m_octreeDim = leafLvlDim;
			uint lvl = leafLvlDim.m_nodeLvl;
			
			if( isReleasing )
			{
				isReleasing = false;
			}
			
			// Hierarchy construction loop.
			while( lvl )
			{
				// Debug
				{
					lock_guard< mutex > lock( m_logMutex );
					cout << "======== Starting lvl " << lvl << " ========" << endl << endl;
				}
				
				OctreeDim nextLvlDim( m_octreeDim, m_octreeDim.m_nodeLvl - 1 );
				
				while( true )
				{
					size_t workListSize;
					{
						// This lock is need so m_workList.size() access is not optimized, returning outdated results.
						lock_guard< mutex > lock( m_listMutex );
						workListSize = m_lvlWorkLists[ lvl ].size();
					}
					
					if( workListSize > 0 )
					{
						// Debug
						{
							lock_guard< mutex > lock( m_logMutex );
							cout << "iter start" << endl << endl;
						}
						
						int dispatchedThreads = ( workListSize > M_N_THREADS ) ? M_N_THREADS : workListSize;
						IterArray iterInput( dispatchedThreads );
						
						// TODO: The last sibling group cannot be processed before the last iteration, since its remaining
						// nodes reading can still be pending.
						for( int i = dispatchedThreads - 1; i > -1; --i )
						{
							iterInput[ i ] = popWork( lvl );
							
							// Debug
	// 						{
	// 							cout << "t " << i << " size: " << iterInput[ i ].size() << endl << endl; 
	// 						}
						}
						
						IterArray iterOutput( dispatchedThreads );
						
						#pragma omp parallel for
						for( int i = 0; i < dispatchedThreads; ++i )
						{
							NodeList& input = iterInput[ i ];
							NodeList& output = iterOutput[ i ];
							bool isBoundarySiblingGroup = true;
							
							while( !input.empty() )
							{
								Node& node = input.front();
								Morton parentCode = *m_octreeDim.calcMorton( node ).traverseUp();
								
								NodeArray siblings( 8 );
								siblings[ 0 ] = std::move( node );
								input.pop_front();
								int nSiblings = 1;
								
								// Debug
								{
									lock_guard< mutex > lock( m_logMutex );
	 								cout << "Thread " << omp_get_thread_num() << " sibling [ 0 ]: "
										 << m_octreeDim.calcMorton( siblings[ 0 ] ).getPathToRoot( true );
	 							}
								//
								
								while( !input.empty() && *m_octreeDim.calcMorton( input.front() ).traverseUp() == parentCode )
								{
									siblings[ nSiblings ] = std::move( input.front() );
									++nSiblings;
									input.pop_front();
									
									// Debug
									{
										lock_guard< mutex > lock( m_logMutex );
										cout << "Thread " << omp_get_thread_num() << " sibling [ " << nSiblings - 1 << " ]: "
											<< m_octreeDim.calcMorton( siblings[ nSiblings - 1 ] ).getPathToRoot( true );
									}
								}
								
								if( input.empty() )
								{
									isBoundarySiblingGroup = true;
								}
								
								// Debug
	// 							{
	// 								lock_guard< mutex > lock( logMutex );
	// 								cout << "Thread " << omp_get_thread_num() << " nSiblings: " << nSiblings
	// 									 << " is leaf: " << siblings[ 0 ].isLeaf() << " is boundary: "
	// 									 << isBoundarySiblingGroup << endl << endl;
	// 							}
								
								if( nSiblings == 1 && siblings[ 0 ].isLeaf() && !isBoundarySiblingGroup )
								{
									// Debug
									{
										lock_guard< mutex > lock( m_logMutex );
										cout << "Thread " << omp_get_thread_num() << " collapse." << endl << endl;
									}
									
									// Collapse: just put the node to be processed in next level.
									output.push_back( std::move( siblings[ 0 ] ) );
								}
								else
								{
									// Debug
									{
										lock_guard< mutex > lock( m_logMutex );
										cout << "Thread " << omp_get_thread_num() << " LOD." << endl << endl;
									}
									
									// LOD
									Node inner = createInnerNode( std::move( siblings ), nSiblings );
									
									if( isReleasing && !isBoundarySiblingGroup )
									{
										// Debug
										{
											lock_guard< mutex > lock( m_logMutex );
											cout << "Thread " << omp_get_thread_num() << " will release" << endl << endl;
										}
										
										releaseSiblings( inner.child(), omp_get_thread_num(), m_octreeDim );
										
										// Debug
										{
											cout << "MEMORY AFTER RELEASE: " << AllocStatistics::totalAllocated() << endl << endl;
										}
									}
									
									output.push_back( std::move( inner ) );
									isBoundarySiblingGroup = false;
								}
							}
						}
						
						// Debug
	// 					{
	// 						cout << "Before work merge:" << endl;
	// 						for( int i = 0; i < iterOutput.size(); ++i )
	// 						{
	// 							cout << "output " << i << endl
	// 								 << nodeListToString( iterOutput[ i ], nextLvlDim );
	// 						}
	// 						cout << endl;
	// 					}
						
						
						WorkList& nextLvlWorkList = m_lvlWorkLists[ lvl - 1 ];
						
						if( !nextLvlWorkList.empty() )
						{
							// Debug
	// 						{
	// 							cout << "Merging nextLvl back and output " << dispatchedThreads - 1 << endl << endl;
	// 						}
							
							NodeList nextLvlBack = std::move( nextLvlWorkList.back() );
							nextLvlWorkList.pop_back();
							
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
						if( !nextLvlWorkList.empty() )
						{
							removeBoundaryDuplicate( nextLvlWorkList.back(), iterOutput[ 0 ], nextLvlDim );
						}
						nextLvlWorkList.push_back( std::move( iterOutput[ 0 ] ) );
						
						// Debug
	// 					{
	// 						bool started = false;
	// 						Node* prevNode;
	// 						cout << "next lvl after merge " << endl;
	// 						for( NodeList& work : m_nextLvlWorkList )
	// 						{
	// 							cout << "nodelist " << endl;
	// 							for( Node& node : work )
	// 							{
	// 								if( started )
	// 								{
	// 									Morton prevMorton = nextLvlDim.calcMorton( *prevNode );
	// 									Morton nodeMorton = nextLvlDim.calcMorton( node );
	// 									cout << "prev: " << prevMorton.getPathToRoot( true ) << "node: "
	// 										 << nodeMorton.getPathToRoot( true ) << endl;
	// 									
	// 									if( ( m_leafLvl - m_octreeDim.m_nodeLvl ) % 2 )
	// 									{
	// 										assert( prevMorton < nodeMorton );
	// 									}
	// 									else
	// 									{
	// 										assert( nodeMorton < prevMorton );
	// 									}
	// 								}
	// 								prevNode = &node;
	// 								started = true;
	// 							}
	// 						}
	// 						cout << endl;
	// 					}
						
						if( isReleasing )
						{
							if( AllocStatistics::totalAllocated() < m_memoryLimit )
							{
								{
									cout << "===== RELEASE OFF =====" << endl << endl;
								}
								
								isReleasing = false;
								releaseFlag.notify_one();
							}
						}
						else if( AllocStatistics::totalAllocated() > m_memoryLimit )
						{
							// Check memory stress and release memory if necessary.
							
							// Debug
							{
								cout << "===== RELEASE ON =====" << endl << endl;
							}
							
							isReleasing = true;
							//releaseNodes();
						}
					}
					else
					{
						if( leafLvlLoaded )
						{
							// The last NodeList is not collapsed yet.
							collapseBoundaries( m_lvlWorkLists[ lvl - 1 ].back(), nextLvlDim );
						}
						
						break;
					}
				}
					
				--lvl;
				
				// Debug
	// 			{
	// 				cout << "Prev dim: " << m_octreeDim << endl;
	// 			}
				//
				
				m_octreeDim = OctreeDim( m_octreeDim, lvl );
				
				// Debug
	// 			{
	// 				cout << "Current dim: " << m_octreeDim << endl
	// 					 << "Lvl processed: " << endl
	// 					 << workListToString( m_nextLvlWorkList, m_octreeDim );
	// 			}
				//
			}
		}
		
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
		{
			cout << "Thread " << threadIdx << " n siblings: " << siblings.size() << endl << endl;
		}
		
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
			{
				lock_guard< mutex > lock( m_logMutex );
				cout << "Thread " << threadIdx << ": persisting sibling " << i << ": "
					 << siblingMorton.getPathToRoot( true ) << endl << endl;
			}
			
			// Persisting node
			sql.insertNode( siblingMorton, sibling );
		}
		
		siblings.clear();
	}
	
	template< typename Morton, typename Point >
	inline void HierarchyCreator< Morton, Point >::releaseNodes()
	{
		// Debug
		{
			cout << "Mem before release: " << AllocStatistics::totalAllocated() << endl << endl;
		}
		
		// IMPORTANT: Nodes in next lvl work list back cannot be released because they can have pendent collapses.
		int releaseLvl = m_octreeDim.m_nodeLvl - 1;
		auto workListIt = std::next( m_lvlWorkLists[ releaseLvl ].rbegin() );
		
		// Debug
// 		{
// 			OctreeDim dim( m_octreeDim, releaseLvl );
// 			cout << "Skiped worklist: " << nodeListToString( *m_nextLvlWorkList.rbegin(), dim ) << endl;
// 		}
		
		while( AllocStatistics::totalAllocated() > m_memoryLimit )
		{
			// The strategy is to release nodes without harming of the next hierarchy creation iterations. So, any current
			// lvl unprocessed and next lvl nodes are spared. Thus, the passes are:
			// 1) Release children nodes of the next-lvl worklist.
			// 2) Release children nodes of current-lvl worklist.
			
			int dispatchedThreads = 0;
			Array< NodeList* > iterArray( M_N_THREADS );
			
			while( dispatchedThreads < M_N_THREADS && workListIt != m_lvlWorkLists[ releaseLvl ].rend() )
			{
				iterArray[ dispatchedThreads++ ] = &*( workListIt++ );
			}
			
			#pragma omp parallel for
			for( int i = 0; i < dispatchedThreads; ++i )
			{
				int threadIdx = omp_get_thread_num();
				
				Sql& sql = m_dbs[ i ];
				sql.beginTransaction();
				
				NodeList& nodeList = *iterArray[ i ];
				for( Node& node : nodeList )
				{
					// Debug
// 					{
// 						OctreeDim dim( m_octreeDim, releaseLvl );
// 						lock_guard< mutex > lock( m_logMutex );
// 						if( dim.m_nodeLvl == m_octreeDim.m_nodeLvl - 1 )
// 						{
// 							cout << "Thread " << threadIdx << ": Next lvl release: children of "
// 								 << dim.calcMorton( node ).getPathToRoot( true ) << endl;
// 						}
// 						else
// 						{
// 							cout << "Thread " << omp_get_thread_num() <<  ": Current Lvl release: children of "
// 									<< dim.calcMorton( node ).getPathToRoot( true ) << endl;
// 						}
// 					}
					
					if( !node.child().empty() )
					{
						// Debug
// 						{
// 							lock_guard< mutex > lock( m_logMutex );
// 							cout << "Thread " << omp_get_thread_num() <<  ": Has children" << endl << endl;
// 						}
						
						releaseSiblings( node.child(), threadIdx, OctreeDim( m_octreeDim, releaseLvl + 1 ) );
					}
				}
				
				sql.endTransaction();
			}
			
			if( releaseLvl == m_octreeDim.m_nodeLvl )
			{
				if( workListIt == m_lvlWorkLists[ releaseLvl ].rend() )
				{
					break;
				}
			}
			else if( workListIt == m_lvlWorkLists[ releaseLvl ].rend() )
			{
				releaseLvl += 1;
				workListIt = m_lvlWorkLists[ releaseLvl ].rbegin();
			}
		}
		
		// Debug
		{
			cout << "Visited all nodes: " <<  bool( workListIt == m_lvlWorkLists[ releaseLvl ].rend() ) << endl
				 << "Mem after release: " << AllocStatistics::totalAllocated() << endl << endl;
		}
	}
	
	template< typename Morton, typename Point >
	inline typename HierarchyCreator< Morton, Point >::Node HierarchyCreator< Morton, Point >
	::createInnerNode( NodeArray&& inChildren, uint nChildren ) const
	{
		if( inChildren.size() == 1 )
		{
			// Set parental link in grand-children.
			Node& child = inChildren[ 0 ];
			if( !child.isLeaf() )
			{
				setParent( child );
			}
			
			Node node( child.getContents(), false );
			node.setChildren( std::move( inChildren ) );
			
			return node;
		}
		else
		{
			using Map = map< int, Node&, less< int >, ManagedAllocator< pair< const int, Node& > > >;
			using MapEntry = typename Map::value_type;
			
			// Depending on lvl, the sibling vector can be reversed.
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