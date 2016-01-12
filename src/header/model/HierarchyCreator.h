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
		m_octreeDim( dim ),
		m_leafLvl( m_octreeDim.m_nodeLvl ),
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
		void swapWorkLists()
		{
			m_workList = std::move( m_nextLvlWorkList );
			m_nextLvlWorkList = WorkList();
		}
		
		// TODO: The lock can be avoided after the disk thread has finished its work.
		void pushWork( NodeList&& workItem )
		{
			lock_guard< mutex > lock( m_listMutex );
			m_workList.push_back( std::move( workItem ) );
		}
		
		// TODO: The lock can be avoided after the disk thread has finished its work.
		NodeList popWork()
		{
			lock_guard< mutex > lock( m_listMutex );
			NodeList nodeList = std::move( m_workList.front() );
			m_workList.pop_front();
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
			
			Node& prevFirstNode = previousProcessed.front();
			Node& nextLastNode = nextProcessed.back();
			
			if( nextLvlDim.calcMorton( prevFirstNode ) == nextLvlDim.calcMorton( nextLastNode ) )
			{
				// Nodes from same sibling groups were in different threads
				// TODO: the merge operation needs to select again the points in nextLastNode.
				
				NodeArray& prevFirstNodeChild = prevFirstNode.child();
				NodeArray& nextLastNodeChild = nextLastNode.child();
				
				// Debug
// 				{
// 					cout << "Prev merged child: " << m_octreeDim.calcMorton( prevFirstNodeChild[ 0 ] ).getPathToRoot( true )
// 						 << endl << "Next merged child: "
// 						 << m_octreeDim.calcMorton( nextLastNodeChild[ 0 ] ).getPathToRoot( true ) << endl;
// 				}
				
				NodeArray mergedChild( prevFirstNodeChild.size() + nextLastNodeChild.size() );
				
				NodeArray* lesserMortonChild;
				NodeArray* greaterMortonChild;
				
				if( ( m_leafLvl - m_octreeDim.m_nodeLvl ) % 2 )
				{
					// prev nodes have greater morton
					lesserMortonChild = &nextLastNodeChild;
					greaterMortonChild = &prevFirstNodeChild;
				}
				else
				{
					// prev nodes have lesser morton
					lesserMortonChild = &prevFirstNodeChild;
					greaterMortonChild = &nextLastNodeChild;
				}
				
				// Debug
// 				{
// 					cout << "Less merged child: " << m_octreeDim.calcMorton( ( *lesserMortonChild )[ 0 ] ).getPathToRoot( true )
// 						 << endl << "Great merged child: "
// 						 << m_octreeDim.calcMorton( ( *greaterMortonChild )[ 0 ] ).getPathToRoot( true ) << endl;
// 				}
				
				//Debug
// 				{
// 					Morton duplicateCode = nextLvlDim.calcMorton( previousProcessed.front() );
// 					cout << "Same sibling group in different threads. Removing duplicate:" << endl
// 						 << duplicateCode.getPathToRoot( true ) << "resulting node size: "
// 						 << lesserMortonChild->size() + greaterMortonChild->size() << endl << endl;
// 				}
				
				for( int i = 0; i < lesserMortonChild->size(); ++i )
				{
					mergedChild[ i ] = std::move( ( *lesserMortonChild )[ i ] );
					// Parent pointer needs to be fixed.
					setParent( mergedChild[ i ] );
				}
				for( int i = 0; i < greaterMortonChild->size(); ++i )
				{
					int idx = lesserMortonChild->size() + i;
					mergedChild[ idx ] = std::move( ( *greaterMortonChild )[ i ] );
					// Parent pointer needs to be fixed.
					setParent( mergedChild[ idx ] );
				}
				
				nextLastNode.setChildren( std::move( mergedChild ) );
				
				previousProcessed.pop_front();
			}
			
			collapseBoundaries( previousProcessed, nextLvlDim );
			
			// If needed, collapse the last node of nextProcessed
			NodeArray& nextLastNodeChild = nextLastNode.child();
			if( nextLastNodeChild.size() == 1 && nextLastNodeChild[ 0 ].isLeaf() )
			{
				nextLastNode.turnLeaf();
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
				
				nextProcessed.splice( nextProcessed.end(), previousProcessed );
			}
			else
			{
				// Debug
// 				{
// 					cout << "Pushing next to global worklist." << endl << endl;
// 				}
				
				if( !m_nextLvlWorkList.empty() )
				{
					removeBoundaryDuplicate( m_nextLvlWorkList.front(), previousProcessed, nextLvlDim );
				}
				
				m_nextLvlWorkList.push_front( std::move( previousProcessed ) );
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
		
		/** Holds the work list with nodes of the lvl above the current one. */
		WorkList m_nextLvlWorkList;
		
		/** SHARED. Holds the work list with nodes of the current lvl. Database thread and master thread have access. */
		WorkList m_workList;
		
		/** Current lvl octree dimensions. */
		OctreeDim m_octreeDim;
		/** Leaf lvl. */
		uint m_leafLvl;
		
		string m_plyFilename;
		
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
		// SHARED. The disk access thread sets this true when it finishes reading all points in the sorted file.
		bool leaflvlFinished = false;
		
		mutex releaseMutex;
		bool isReleasing = false;
		// SHARED. Indicates when a node release is ocurring.
		condition_variable releaseFlag;
		
		// Thread that loads data from sorted file or database.
		thread diskAccessThread(
			[ & ]()
			{
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
							Morton code = m_octreeDim.calcMorton( p );
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
				
				leaflvlFinished = true;
			}
		);
		diskAccessThread.detach();
		
		uint lvl = m_octreeDim.m_nodeLvl;
		
		// Hierarchy construction loop.
		while( lvl )
		{
			cout << "======== Starting lvl " << lvl << " ========" << endl << endl;
			
			OctreeDim nextLvlDim( m_octreeDim, m_octreeDim.m_nodeLvl - 1 );
			
			while( true )
			{
				size_t workListSize;
				{
					// This lock is need so m_workList.size() access is not optimized, returning outdated results.
					lock_guard< mutex > lock( m_listMutex );
					workListSize = m_workList.size();
				}
				
				if( workListSize > 0 )
				{
					// Debug
// 					{
// 						cout << "iter start" << endl << endl;
// 					}
					
					int dispatchedThreads = ( workListSize > M_N_THREADS ) ? M_N_THREADS : workListSize;
					IterArray iterInput( dispatchedThreads );
					
					for( int i = dispatchedThreads - 1; i > -1; --i )
					{
						iterInput[ i ] = popWork();
						
						// Debug
						{
							cout << "t " << i << " size: " << iterInput[ i ].size() << endl << endl; 
						}
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
// 							{
// 								lock_guard< mutex > lock( m_logMutex );
//  								cout << "Thread " << omp_get_thread_num() << " sibling [ 0 ]: "
// 									 << m_octreeDim.calcMorton( siblings[ 0 ] ).getPathToRoot( true );
//  							}
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
// 								{
// 									lock_guard< mutex > lock( logMutex );
// 									cout << "Thread " << omp_get_thread_num() << " collapse." << endl << endl;
// 								}
								
								// Collapse: just put the node to be processed in next level.
								output.push_front( std::move( siblings[ 0 ] ) );
							}
							else
							{
								// Debug
// 								{
// 									lock_guard< mutex > lock( logMutex );
// 									cout << "Thread " << omp_get_thread_num() << " LOD." << endl << endl;
// 								}
								
								// LOD
								Node inner = createInnerNode( std::move( siblings ), nSiblings );
								output.push_front( std::move( inner ) );
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
					
					
					if( !m_nextLvlWorkList.empty() )
					{
						// Debug
// 						{
// 							cout << "Merging nextLvl front and output " << dispatchedThreads - 1 << endl << endl;
// 						}
						
						NodeList nextLvlFront = std::move( m_nextLvlWorkList.front() );
						m_nextLvlWorkList.pop_front();
						
						mergeOrPushWork( nextLvlFront, iterOutput[ dispatchedThreads - 1 ], nextLvlDim );
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
					if( !m_nextLvlWorkList.empty() )
					{
						removeBoundaryDuplicate( m_nextLvlWorkList.front(), iterOutput[ 0 ], nextLvlDim );
					}
					m_nextLvlWorkList.push_front( std::move( iterOutput[ 0 ] ) );
					
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
					
					// Check memory stress and release memory if necessary.
					if( AllocStatistics::totalAllocated() > m_memoryLimit )
					{
						// Debug
// 						{
// 							cout << "===== RELEASE TRIGGERED =====" << endl << endl;
// 						}
						
						isReleasing = true;
						releaseNodes();
						isReleasing = false;
						releaseFlag.notify_one();
					}
				}
				else
				{
					if( leaflvlFinished )
					{
						// The last NodeList is not collapsed yet.
						collapseBoundaries( m_nextLvlWorkList.front(), nextLvlDim );
						break;
					}
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
			
			swapWorkLists();
		}
		
		Node* root = new Node( std::move( m_workList.front().front() ) );
		
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
	inline void HierarchyCreator< Morton, Point >::releaseNodes()
	{
		auto workListIt = m_nextLvlWorkList.rbegin();
		int releaseLvl = m_octreeDim.m_nodeLvl - 1;
		
		while( AllocStatistics::totalAllocated() > m_memoryLimit )
		{
			// The strategy is to release nodes without harming of the next hierarchy creation iterations. So, any current
			// lvl unprocessed and next lvl nodes are spared. Thus, the passes are:
			// 1) Release children nodes of the next-lvl worklist.
			// 2) Release children nodes of current-lvl worklist.
			
			int dispatchedThreads = 0;
			Array< NodeList* > iterArray( M_N_THREADS );
			
			if( releaseLvl == m_octreeDim.m_nodeLvl - 1 )
			{
				// IMPORTANT: Nodes in next lvl work list front cannot be released because they can have pendent collapses.
				while( dispatchedThreads < M_N_THREADS && std::next( workListIt ) != m_nextLvlWorkList.rend() )
				{
					iterArray[ dispatchedThreads++ ] = &*( workListIt++ );
				}
			}
			else
			{
				while( dispatchedThreads < M_N_THREADS && workListIt != m_workList.rend() )
				{
					iterArray[ dispatchedThreads++ ] = &*( workListIt++ );
				}
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
			
			if( workListIt == m_workList.rend() )
			{
				break;
			}
			else if( std::next( workListIt ) == m_nextLvlWorkList.rend() )
			{
				// Debug
// 				{
// 					OctreeDim dim( m_octreeDim, releaseLvl );
// 					cout << "Skiped worklist: " << nodeListToString( *workListIt, dim ) << endl;
// 				}
				
				releaseLvl += 1;
				workListIt = m_workList.rbegin();
			}
		}
		
		// Debug
		{
			cout << "Used mem: " << AllocStatistics::totalAllocated() << endl << endl;
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
			if( ( m_leafLvl - m_octreeDim.m_nodeLvl ) % 2 )
			{
				for( int i = 0; i < children.size(); ++i )
				{
					children[ i ] = std::move( inChildren[ children.size() - 1 - i ] );
				}
			}
			else
			{
				for( int i = 0; i < children.size(); ++i )
				{
					children[ i ] = std::move( inChildren[ i ] );
				}
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