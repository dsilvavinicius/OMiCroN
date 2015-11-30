#ifndef O1OCTREE
#define O1OCTREE

#include "PointSorter.h"
#include "O1OctreeNode.h"

namespace model
{
	/** Out-of-core fast parallel octree. */
	template< typename Morton, typename Point >
	class FastParallelOctree
	{
	public:
		using PointPtr = shared_ptr< Point >;
		using Node = O1OctreeNode< PointPtr >;

		FastParallelOctree() : M_N_THREADS( 8 ) {}
		
		/**
		 * Builds from a .ply file, generates a sorted file in the process which can be used with buildFromSortedFile()
		 * later on to skip sorting, increasing creation performance.
		 * @param maxLvl is the level from which the octree will be constructed bottom-up. Lesser values incur in
		 * less created nodes, but also less possibilities for LOD ( level of detail ). In practice, the more points the
		 * model has, the deeper the hierachy needs to be for good visualization. */
		void buildFromFile( const string& plyFileName, const int& maxLvl );
		
		/** Builds from a .ply file. The file is assumed to be sorted in z-order. Use buildFromFile() to generate the
		 * sorted .ply file. */
		void buildFromSortedFile( const string& plyFileName );
	
		/** Gets dimensional info of this octree. */
		const OctreeDimensions& dims() const;
		
		template< typename M, typename P >
		friend ostream& operator<<( ostream& out, const FastParallelOctree< M, P >& octree );
		
	private:
		/** Creates all nodes bottom-up. In any moment, nodes can be released and loaded from database in order to make
		 * good use of memory.*/
		void buildHierarchy();
		
		void setupNodeRendering( Node node, RenderingState& renderingState );
		
		/** Dimensional info of this octree. */
		OctreeDimensions m_dim;
		
		/** Root node of the hierarchy. */
		Node m_root;
		
		/** Number of threads used in octree construction and front tracking. The database thread is not part of the
		 * group. */
		constexpr int M_N_THREADS;
	};
	
	template< typename Morton, typename Point >
	void FastParallelOctree< Morton, Point >::buildFromFile( const string& plyFileName, const int& maxLvl )
	{
		PointSorter< Morton, Point > sorter( plyFileName, maxLvl );
		
		string sortedFileName = plyFileName;
		sortedFileName.insert( 0, "sorted_" );
		sorter.sort( sortedFileName );
		m_dim = sorter.comp();
		
		buildFromSortedFile( sortedFileName );
	}
	
	template< typename Morton, typename Point >
	void FastParallelOctree< Morton, Point >::buildFromSortedFile( const string& plyFileName )
	{
		using Direction = bool;
		using RIGHT = true;
		using LEFT = false;
		
		// List of nodes. Each list should be processed by a thread.
		using NodeList = list< Node, ManagedAllocator< Node > >;
		// List of all NodeLists.
		using WorkList = list< NodeList, ManagedAllocator< Node > >;
		// Lists that will be processed in current iteration.
		using IterLists = vector< NodeList, ManagedAllocator< NodeList > >;
		
		// Thread that loads data from sorted file or database.
		thread databaseThread = thread(
			[ & ]()
			{
				
			}
		);
		
		Direction direction = RIGHT;
		
		WorkList nextLvlWorkLists;
		WorkList workList; // SHARED. Database thread will be filling this with work.
		
		uint lvl = m_dim.m_leafLvl;
		
		while( lvl )
			while( !databaseThread.lvlFinished() )
			{
				if( workList.size > 0 )
				{
					int dispatchedThreads = ( workList.size > M_N_THREADS ) ? M_N_THREADS : workLists.size;
					IterLists iterLists( dispatchedThreads );
					
					for( int i = dispatchedThreads - 1 to 0 )
					{
						iterLists[ i ] = workList.front();
						workList.pop_front();
					}
						
					#pragma omp parallel for
					for( int i = 0; i < M_N_THREADS; ++i )
					{
						WorkList currentProcessed;
					}

					for( int i = dispatchedThreads - 1 to 1 )
						mergeOrPushWork( threads[ i - 1 ].currentProcessed, threads[ i ].currentProcessed )

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
	
	Thread
	workList currentProcessed // All nodes processed by current iteration 

	run( worksList )
		processWork( works )

	processWork( workList )
		while( !workList.empty )
			node = workList.pop_front().node
			parent = node.calcMorton().parent()
			
			siblings[ 0 ] = node
			nSiblings = 1
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

MasterThread : Thread
	list< Worklist > nextLvlWorkLists
	list< WorkList > workLists // SHARED
	ended
	lvlEnded
	EXPECTED_LOAD_PER_THREAD
	MAX_WORK_LISTS

	thread threads[ nThreads ] // other slave threads

	swapWorkLists()
		workLists = move( nextLvlWorkLists )
		nextLvlWorkLists = list< WorkList >

	pushWork( workList )
		queue_lock
			workLists.push_back( workList )

	popWork()
		return workLists.pop_front()

	// Merge nextProcessed into previousProcessed if there is not enough work yet to form a WorkList or push it to nextLvlWorkLists
	// otherwise. Repetitions are checked while linking lists, since this case can occur when the lists have nodes from the same 
	// sibling group.
	mergeOrPushWork( previousProcessed, nextProcessed )
		if( nextProcessed.size < EXPECTED_LOAD_PER_THREAD )
			if( previousProcessed.end.prev == nextProcessed.begin )
				// Nodes from same sibling groups were in different threads
				previousProcessed.end.prev.erase() 
			previousProcessed.end.link( next.begin )
		else
			nextLvlWorkLists.push_front( nextProcessed )

	run()
		if( !ended )
			if( workLists.size > 0 )
				dispatchedThreads = ( workLists.size > nThreads + 1 ) ? nThreads + 1 : workLists.size
				for( int i = dispatchedThreads - 1 to 0 )
					workList = popWork()
					thread[ i ].run( workList )

				workList = popWork()
				processWork( workList )
				
				waitOthersFinish()

				for( int i = dispatchedThreads - 1 to 1 )
					mergeOrPushWork( threads[ i - 1 ].currentProcessed, threads[ i ].currentProcessed )

				mergeOrPushWork( currentProcessed, threads[ 0 ].currentProcessed )
				mergeOrPushWork( nextLvlWorkLists.front, currentProcessed ) 
				
				nextLvlWorkLists.push_front( currentProcessed )
			else if( lvlEnded )
					swapWorkLists()
				else
					wait()

readLvlChunk( direction, EXPECTED_LOAD_PER_THREAD )
	query = queryDB()
	if( direction = right )
		while( workLists.size < nThreads + 1 AND !query.empty() )
			while( workList.size < EXPECTED_LOAD_PER_THREAD AND !query.empty() )
				workList.push_back( query.step() )
			workLists.push_back( workList )
	else
		while( workLists.size < nThreads + 1 AND !query.empty() )
			while( workList.size < EXPECTED_LOAD_PER_THREAD AND !query.empty() )
				workList.push_front( query.step() )
			workLists.push_front( workList )

createLvls()
	direction = right
	while( true )
		if( lvl == root )
			masterThread.ended = true
			return

		lvlEnded = false
		while( !lvlEnded )
			while( masterThread.workLists.size < MAX_WORK_LISTS )
				// Assumed to sort works in the current direction
				list< WorkList > works = readLvlChunk( direction, EXPECTED_LOAD_PER_THREAD )
				if( works.empty )
					lvlEnded = true
					masterThread.lvlEnded = true
					break

				masterThread.pushWork( works )

				if( notEnoughMem )
					// When persising, should also delete children in case that the node is leaf ( merging )
					// Also, lastDirty must be updated if applyable
					persistAndRelease( masterThread.workLists, direction )
		riseLvl()
		toggleDirection( direction )

createHierarchy()
	readAndSortPoints()
	createLvls()
}

#endif