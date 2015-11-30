#ifndef HIERARCHY_CREATOR_H
#define HIERARCHY_CREATOR_H

#include <mutex>
#include <condition_variable>
#include "ManagedAllocator.h"
#include <O1OctreeNode.h>
#include <OctreeDimensions.h>

namespace model
{
	/** Manages all FastParallelOctree hierarchy creation. */
	template< typename Morton, typename Point >
	class HierarchyCreator
	{
	public:
		using PointPtr = shared_ptr< Point >;
		using PointVector = vector< PointPtr >;
		using Node = O1OctreeNode< PointPtr >;
		
		using OctreeDim = OctreeDimensions< Morton, Point >;
		using Reader = PlyPointReader< Point >;
		
		/** List of nodes that can be processed parallel by one thread. */
		using NodeList = list< Node, ManagedAllocator< Node > >;
		// List of NodeLists.
		using WorkList = list< NodeList, ManagedAllocator< Node > >;
		// Vector with lists that will be processed in current iteration.
		using IterVector = vector< NodeList, ManagedAllocator< NodeList > >;
		
		/** Direction of lvl processing. */
		enum Direction
		{
			RIGHT = true,
			LEFT = false
		};
		
		HierarchyCreator() {}
		
		/** Creates the hierarchy from a sorted file.
		 * @return hierarchy's root node. */
		Node createFromSortedFile( const string& plyFileName, const size_t maxUsedMemory, const OctreeDim& dim );
		
	private:
		void swapWorkLists()
		{
			m_workList = std::move( m_nextLvlWorkList );
			m_nextLvlWorkList = WorkList();
		}
		
		void pushWork( NodeList&& workItem )
		{
			lock_guard< mutex >( m_listMutex );
			m_workList.push_back( workItem );
		}
		
		NodeList popWork()
		{
			lock_guard< mutex >( m_listMutex );
			NodeList nodeList = std::move( m_workList.front() );
			m_workList.pop_front();
			return nodeList;
		}
		
		/** Merge nextProcessed into previousProcessed if there is not enough work yet to form a WorkList or push it to
		 * nextLvlWorkLists otherwise. Repetitions are checked while linking lists, since this case can occur when the
		 * lists have nodes from the same sibling group. */
		void mergeOrPushWork( NodeList& previousProcessed, const NodeList& nextProcessed )
		{
			if( nextProcessed.size() < m_expectedLoadPerThread )
			{
				if( *( --previousProcessed.end() ) == nextProcessed.begin() )
				{
					// Nodes from same sibling groups were in different threads
					previousProcessed.pop_back();
				}
				previousProcessed.splice( previsouProcessed.end(), nextProcessed );
			}
			else
			{
				m_nextLvlWorkList.push_front( nextProcessed );
			}
		}
				
		/** Holds the work list with nodes of the lvl above the current one. */
		WorkList m_nextLvlWorkList;
		
		/** SHARED. Holds the work list with nodes of the current lvl. Database thread and master thread have access. */
		WorkList m_workList;
		
		/** m_lastPersisted[ i ] contains the last persisted Morton in lvl i. */
		vector< MortonCode > m_lastPersisted;
		
		/** Mutex for the work list. */
		mutex m_listMutex;
		
		ulong m_expectedLoadPerThread;
		
		constexpr int M_N_THREADS;
	};
	
	template< typename Morton, typename Point >
	Node HierarchyCreator< Morton, Point >::
	createFromSortedFile( const string& plyFileName, const size_t maxUsedMemory, const OctreeDim& dim )
	{
		/** SHARED. true if current level is finished, false otherwise. */
		bool lvlFinished;
		
		Direction direction = RIGHT;
		
		mutex releaseMutex;
		bool isReleasing = false;
		condition_variable releaseFlag;
		
		// Thread that loads data from sorted file or database.
		thread diskAccessThread(
			[ & ]()
			{
				NodeList nodeList;
				PointVector points;
				
				bool releaseNeeded = false;
				Morton currentParent;
				Reader reader( plyFileName );
				reader.read( Reader::SINGLE,
					[ & ]( const Point& p )
					{
						if( isReleasing )
						{
							unique_lock< mutex > lock( releaseMutex );
								m_releaseFlag.wait( lock, []{ return !isReleasing; } );
							
							points.push_back( makeManaged< Point >( p ) );
						}
						else 
						{
							if( AllocStatistics::totalAllocated() < maxUsedMemory )
							{
								Morton code = dim.calcMorton( p );
								Morton parent = code.getParent();
								
								if( parent != currentParent )
								{
									nodeList.push_back( Node( std::move( points ), true ) );
									points = PointVector();
									
									if( nodeList.size() == m_expectedLoadPerThread )
									{
										pushWork( nodeList );
									}
								}
								
								points.push_back( makeManaged< Point >( p ) );
									
								if( releaseNeeded )
								{
									isReleasing = true;
									releaseNeeded = false;
								}
							}
							else
							{
								releaseNeeded = true;
							}
						}
					}
				);
				
				lvlFinished = true;
			}
		);
		
		uint lvl = dim.m_leafLvl;
		
		while( lvl )
			while( !lvlFinished )
			{
				if( m_workList.size > 0 )
				{
					int dispatchedThreads = ( m_workList.size() > M_N_THREADS ) ? M_N_THREADS : m_workList.size();
					IterVector iterInput( dispatchedThreads );
					
					for( int i = dispatchedThreads - 1; i > -1; --i )
					{
						iterInput[ i ] = popWork();
					}
					
					IterVector iterOutput( dispatchedThreads );
					
					#pragma omp parallel for
					for( int i = 0; i < M_N_THREADS; ++i )
					{
						NodeList& input = iterInput[ i ];
						NodeList& output = iterOutput[ i ];
						while( !input.empty() )
						{
							Node& node = input.front();
							Morton parentCode = dim.calcMorton( node ).getParent();
							
							vector< Node > siblings( 8 );
							siblings[ 0 ] = node;
							int nSiblings = 1;
							
							// CONTINUE HERE!!!
							
							while( workList.front().node.calcMorton().parent() == parent )
								++nSiblings
								node = workList.pop_front().node
								siblings[ nSiblings ] = node
							
							if( nSiblings == 1 )
								// Merging, just put the node to be processed in next level.
								currentProcessed.push_front( siblings[ 0 ] )
							else
								// LOD
								siblingGroup = createSiblingGroup( siblings )
								// Create the new inner, with correct pointers and fix parent pointers for children
								inner = newInner( siblingGroup ) 
								currentProcessed.push_front( inner )
						}
					}

					for( int i = dispatchedThreads - 1; i > 0; --i )
					{
						mergeOrPushWork( threads[ i - 1 ].currentProcessed, threads[ i ].currentProcessed )
					}

					mergeOrPushWork( currentProcessed, threads[ 0 ].currentProcessed )
					mergeOrPushWork( nextLvlWorkLists.front, currentProcessed ) 
					
					nextLvlWorkLists.push_front( currentProcessed )
				else 
					wait()
				}
			}
				
			lvl -= 1;
			direction = !direction;
			swapWorkLists()
		}
	}
}

#endif