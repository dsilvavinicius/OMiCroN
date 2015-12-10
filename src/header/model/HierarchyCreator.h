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
		// Array of first-dirty-node-per-level info.
		using DirtyArray = Array< Morton >;
		
		/** Ctor.
		 * @param sortedPlyFilename is a sorted .ply point filename.
		 * @param dim is the OctreeDim of the octree to be constructed.
		 * @param expectedLoadPerThread is the size of the NodeList that will be passed to each thread in the
		 * hierarchy creation loop iterations.
		 * @param memoryLimit is the allowed soft limit of memory consumption by the creation algorithm. */
		HierarchyCreator( const string& sortedPlyFilename, const OctreeDim& dim, ulong expectedLoadPerThread,
						  const size_t memoryLimit )
		: m_plyFilename( sortedPlyFilename ), 
		m_octreeDim( dim ),
		m_leafLvl( m_octreeDim.m_nodeLvl ),
		m_expectedLoadPerThread( expectedLoadPerThread ),
		m_perThreadFirstDirty( M_N_THREADS ),
		m_firstDirty( m_octreeDim.m_nodeLvl + 1 ),
		m_dbs( M_N_THREADS ),
		m_memoryLimit( memoryLimit )
		{
			srand( 1 );
			
			for( int i = 0; i < M_N_THREADS; ++i )
			{
				m_dbs[ i ].init( m_plyFilename.substr( 0, m_plyFilename.find_last_of( "." ) ).append( ".db" ) );
				m_perThreadFirstDirty[ i ] = DirtyArray( m_leafLvl + 1 );
			}
			
			for( int i = m_leafLvl; i > 0; --i )
			{
				if( ( m_leafLvl - i ) % 2 )
				{
					m_firstDirty[ i ] = Morton::getLvlLast( i );
				}
				else
				{
					m_firstDirty[ i ] = Morton::getLvlFirst( i );
				}
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
		
		void pushWork( NodeList&& workItem )
		{
			lock_guard< mutex > lock( m_listMutex );
			m_workList.push_back( std::move( workItem ) );
		}
		
		NodeList popWork()
		{
			lock_guard< mutex > lock( m_listMutex );
			NodeList nodeList = std::move( m_workList.front() );
			m_workList.pop_front();
			return nodeList;
		}
		
		/** Merge nextProcessed into previousProcessed if there is not enough work yet to form a WorkList or push it to
		 * nextLvlWorkLists otherwise. Repetitions are checked while linking lists, since this case can occur when the
		 * lists have nodes from the same sibling group. */
		void mergeOrPushWork( NodeList& previousProcessed, NodeList& nextProcessed )
		{
			if( nextProcessed.size() < m_expectedLoadPerThread )
			{
				if( m_octreeDim.calcMorton( *( --previousProcessed.end() )->getContents()[ 0 ] ) ==
					m_octreeDim.calcMorton( *nextProcessed.begin()->getContents()[ 0 ] ) )
				{
					// Nodes from same sibling groups were in different threads
					previousProcessed.pop_back();
				}
				previousProcessed.splice( previousProcessed.end(), nextProcessed );
			}
			else
			{
				m_nextLvlWorkList.push_front( std::move( nextProcessed ) );
			}
		}
		
		/** Creates an inner Node, given a children array and the number of meaningfull entries in the array. */
		Node createInnerNode( NodeArray&& inChildren, uint nChildren ) const;
		
		/** Releases the sibling group, persisting it if the persistenceFlag is true.
		 * @param siblings is the sibling group to be released and persisted if it is the case.
		 * @param threadIdx is the index of the executing thread.
		 * @param lvl is the hierarchy level of the sibling group.
		 * @param firstSiblingMorton is the morton code of the first sibling.
		 * @param persistenceFlag is true if the sibling group needs to be persisted too. */
		void releaseAndPersistSiblings( NodeArray& siblings, const int threadIdx, const uint lvl,
										const Morton& firstSiblingMorton, const bool persistenceFlag );
		
		/** Releases a given sibling group. */
		void releaseSiblings( NodeArray& node, const int threadIdx, const uint nodeLvl );
		
		/** Releases nodes in order to ease memory stress. */
		void releaseNodes( uint currentLvl );
		
		/** Thread[ i ] uses database connection m_sql[ i ]. */
		Array< Sql > m_dbs;
		
		/** Holds the work list with nodes of the lvl above the current one. */
		WorkList m_nextLvlWorkList;
		
		/** SHARED. Holds the work list with nodes of the current lvl. Database thread and master thread have access. */
		WorkList m_workList;
		
		/** m_perThreadFirstDirty[ i ] contains the first dirty array for thread i. */
		Array< DirtyArray > m_perThreadFirstDirty;
		
		/** m_firstDirty[ i ] contains the first dirty (i.e. not persisted) Morton in lvl i. */
		DirtyArray m_firstDirty;
		
		/** Current lvl octree dimensions. */
		OctreeDim m_octreeDim;
		/** Leaf lvl. */
		uint m_leafLvl;
		
		string m_plyFilename;
		
		/** Mutex for the work list. */
		mutex m_listMutex;
		
		size_t m_memoryLimit;
		
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
			
			while( true )
			{
				if( m_workList.size() > 0 )
				{
					int dispatchedThreads = ( m_workList.size() > M_N_THREADS ) ? M_N_THREADS : m_workList.size();
					IterArray iterInput( dispatchedThreads );
					
					for( int i = dispatchedThreads - 1; i > -1; --i )
					{
						iterInput[ i ] = popWork();
					}
					
					IterArray iterOutput( dispatchedThreads );
					
					#pragma omp parallel for
					for( int i = 0; i < dispatchedThreads; ++i )
					{
						NodeList& input = iterInput[ i ];
						NodeList& output = iterOutput[ i ];
						while( !input.empty() )
						{
							Node& node = input.front();
							Morton parentCode = *m_octreeDim.calcMorton( *node.getContents()[ 0 ] ).traverseUp();
							
							// Debug
							{
								cout << "Processing: " << m_octreeDim.calcMorton( *node.getContents()[ 0 ] ).getPathToRoot( true )
									 << "Parent: " << parentCode.getPathToRoot( true ) << endl;
							}
							//
							
							NodeArray siblings( 8 );
							siblings[ 0 ] = std::move( node );
							input.pop_front();
							int nSiblings = 1;
							
							while( !input.empty() && *m_octreeDim.calcMorton( *input.front().getContents()[ 0 ] ).traverseUp() == parentCode )
							{
								siblings[ nSiblings ] = std::move( input.front() );
								++nSiblings;
								input.pop_front();
							}	
							
							if( nSiblings == 1 && siblings[ 0 ].isLeaf() )
							{
								// Debug
								{
									cout << "Merging" << endl << endl;
								}
								//
								
								// Merging, just put the node to be processed in next level.
								output.push_front( std::move( siblings[ 0 ] ) );
							}
							else
							{
								{
									cout << "LOD" << endl << endl;
								}
								
								// LOD
								Node inner = createInnerNode( std::move( siblings ), nSiblings );
								output.push_front( std::move( inner ) );
							}
						}
					}

					if( dispatchedThreads > 1 )
					{
						for( int i = dispatchedThreads - 1; i > -1; --i )
						{
							mergeOrPushWork( iterOutput[ i - 1 ], iterOutput[ i ] );
						}
					}
					if( !m_nextLvlWorkList.empty() )
					{
						mergeOrPushWork( m_nextLvlWorkList.front(), iterOutput[ 0 ] );
					}
					else
					{
						m_nextLvlWorkList.push_front( std::move( iterOutput[ 0 ] ) );
					}
					
					// Check memory stress and release memory if necessary.
					if( AllocStatistics::totalAllocated() > m_memoryLimit )
					{
						isReleasing = true;
						releaseNodes( lvl );
						isReleasing = false;
						releaseFlag.notify_one();
					}
				}
				else
				{
					if( leaflvlFinished )
					{
						break;
					}
				}
			}
				
			--lvl;
			
			// Debug
			{
				cout << "Prev dim: " << m_octreeDim << endl;
			}
			//
			
			m_octreeDim = OctreeDim( m_octreeDim, lvl );
			
			// Debug
			{
				cout << "Current dim: " << m_octreeDim << endl;
				
				cout << "Lvl processed: " << endl;
				for( Node& node : m_nextLvlWorkList.front() )
				{
					cout << "Point: " << *node.getContents()[ 0 ] << endl 
						 << "Morton: " << m_octreeDim.calcMorton( *node.getContents()[ 0 ] ).getPathToRoot( true ) << endl;
				}
				cout << endl;
			}
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
	::releaseAndPersistSiblings( NodeArray& siblings, const int threadIdx, const uint lvl,
								 const Morton& firstSiblingMorton, const bool persistenceFlag )
	{
		Sql& sql = m_dbs[ threadIdx ];
		if( persistenceFlag )
		{
			Morton siblingMorton = firstSiblingMorton;
			
			for( int i = 0; i < siblings.size(); ++i )
			{
				if( !siblings[ i ].isLeaf() )
				{
					releaseSiblings( siblings[ i ].child(), threadIdx, lvl + 1 );
				}
				
				// Persisting node
				sql.insertNode( siblingMorton, siblings[ i ] );
				siblingMorton = *siblingMorton.getNext();
			}
			
			// Updating thread dirtiness info.
			DirtyArray& firstDirty = m_perThreadFirstDirty[ threadIdx ];
			
			if( ( m_leafLvl - lvl ) % 2 )
			{
				firstDirty[ lvl ] = *firstSiblingMorton.getPrevious();
			}
			else
			{
				firstDirty[ lvl ] = siblingMorton;
			}
		}
		else
		{
			for( int i = 0; i < siblings.size(); ++i )
			{
				if( !siblings[ i ].isLeaf() )
				{
					releaseSiblings( siblings[ i ].child(), threadIdx, lvl + 1 );
				}
			}
		}
	}
	
	template< typename Morton, typename Point >
	inline void HierarchyCreator< Morton, Point >
	::releaseSiblings( NodeArray& siblings, const int threadIdx, const uint lvl )
	{
		OctreeDim siblingsLvlDim( m_octreeDim.m_origin, m_octreeDim.m_size, lvl );
		Morton firstSiblingMorton = siblingsLvlDim.calcMorton( *siblings[ 0 ].getContents()[ 0 ] );
		
		if( ( m_leafLvl - lvl ) % 2 )
		{
			releaseAndPersistSiblings( siblings, threadIdx, lvl, firstSiblingMorton,
									   firstSiblingMorton < m_firstDirty[ lvl ] );
		}
		else
		{
			releaseAndPersistSiblings( siblings, threadIdx, lvl, firstSiblingMorton,
									   m_firstDirty[ lvl ] < firstSiblingMorton );
		}
		
		siblings.clear();
	}
	
	template< typename Morton, typename Point >
	inline void HierarchyCreator< Morton, Point >::releaseNodes( uint currentLvl )
	{
		while( AllocStatistics::totalAllocated() > m_memoryLimit )
		{
			// The strategy is to release nodes without harming of next hierarchy creation iterations. So, any current
			// lvl unprocessed and next lvl nodes are spared. Thus, the passes are:
			// 1) Release children nodes of the next-lvl worklist.
			// 2) Release children nodes of current-lvl worklist.
			
			int dispatchedThreads = 0;
			IterArray iterArray( M_N_THREADS );
			auto workListIt = m_nextLvlWorkList.rbegin();
			
			if( workListIt != m_nextLvlWorkList.rend() )
			{
				while( dispatchedThreads < M_N_THREADS && workListIt != m_nextLvlWorkList.rend() )
				{
					iterArray[ dispatchedThreads++ ] = *( workListIt++ );
				}
			}
			else
			{
				workListIt = m_workList.rbegin();
				while( dispatchedThreads < M_N_THREADS )
				{
					iterArray[ dispatchedThreads++ ] = *( workListIt++ );
				}
			}
			
			#pragma omp parallel for
			for( int i = 0; i < dispatchedThreads; ++i )
			{
				int threadIdx = i;
				
				// Cleaning up thread dirtiness info.
				for( int j = 1; j <= m_leafLvl; ++j )
				{
					m_perThreadFirstDirty[ i ][ j ].build( 0 );
				}
				
				Sql& sql = m_dbs[ i ];
				sql.beginTransaction();
				
				NodeList& nodeList = iterArray[ i ];
				for( Node& node : nodeList )
				{
					if( !node.isLeaf() )
					{
						releaseSiblings( node.child(), threadIdx, currentLvl );
					}
				}
				
				sql.endTransaction();
			}
			
			// Updating hierarchy dirtiness info.
			for( int i = 0; i < dispatchedThreads; ++i )
			{
				for( int j = 1; j <= m_leafLvl; ++j )
				{
					if( ( m_leafLvl - j ) % 2 )
					{
						m_firstDirty[ j ] = std::min( m_firstDirty[ j ], m_perThreadFirstDirty[ i ][ j ] );
					}
					else
					{
						m_firstDirty[ j ] = std::max( m_firstDirty[ j ], m_perThreadFirstDirty[ i ][ j ] );
					}
				}
			}
		}
	}
	
	template< typename Morton, typename Point >
	inline typename HierarchyCreator< Morton, Point >::Node HierarchyCreator< Morton, Point >
	::createInnerNode( NodeArray&& inChildren, uint nChildren ) const
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
			
			// Debug
			{
				cout << "Child: " << m_octreeDim.calcMorton( *child.getContents()[ 0 ] ).getPathToRoot( true )
					 << "addr: " << &child << endl << endl;
			}
			//
			
			prefixMap.insert( prefixMap.end(), MapEntry( nPoints, child ) );
			nPoints += child.getContents().size();
			
			// Set parental relationship of children.
			if( !child.isLeaf() )
			{
				NodeArray& grandChildren = child.child();
				for( int j = 0; j < grandChildren.size(); ++j )
				{
					Node& grandChild = grandChildren[ j ];
					
					// Debug
					{
						OctreeDim grandChildLvlDim( m_octreeDim, m_octreeDim.m_nodeLvl + 1 );
						cout << "GrandChild: " << grandChildLvlDim.calcMorton( *grandChild.getContents()[ 0 ] ).getPathToRoot( true )
							 << "Setting parent as: " << children.data() + i << endl << endl;
					}
					//
					
					grandChild.setParent( children.data() + i );
				}
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
		
		Node node( selectedPoints, false );
		node.setChildren( std::move( children ) );
		
		return node;
	}
}

#endif