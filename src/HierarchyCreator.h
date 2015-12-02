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
		using Sql = SQLiteManager< Point, Morton, Node >;
		
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
		
		HierarchyCreator( const string& plyFilename, const OctreeDim& dim, ulong expectedLoadPerThread,
						  const size_t memoryLimit )
		: M_N_THREADS( 8 ),
		m_plyFilename( plyFilename ), 
		m_octreeDim( dim ),
		m_expectedLoadPerThread( expectedLoadPerThread ),
		m_firstDirty( dim.m_leafLvl ),
		m_dbs( M_N_THREADS ),
		m_memoryLimit( memoryLimit )
		{
			srand( 1 );
			
			for( int i = 0; i < M_N_THREADS; ++i )
			{
				m_dbs[ i ].init( plyFilename.substr( 0, plyFilename.find_last_of( "." ) ).append( ".db" ) );
			}
			
			for( int i = m_firstDirty.size() - 1; i > -1; --i )
			{
				if( ( m_firstDirty.size() - i ) % 2 )
				{
					m_firstDirty[ i ] = Morton::getLvlLast( i );
				}
				else
				{
					m_firstDirty[ i ] = Morton::getLvlFirst( i );
				}
			}
		}
		
		/** Creates the hierarchy from a sorted file.
		 * @return hierarchy's root node. */
		Node createFromSortedFile();
		
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
				m_nextLvlWorkList.push_front( std::move( nextProcessed ) );
			}
		}
		
		/** Creates an inner Node, given its children. */
		Node createInnerNode( vector< Node >&& children ) const;
		
		/** Releases nodes in order to ease memory stress. */
		releaseNodes( uint currentLvl );
		
		/** Releases a given sibling group. */
		void releaseSiblings( Array< Node >& node, Sql& sql, uint nodeLvl );
		
		/** Thread[ i ] uses database connection m_sql[ i ]. */
		Array< Sql > m_dbs;
		
		/** Holds the work list with nodes of the lvl above the current one. */
		WorkList m_nextLvlWorkList;
		
		/** SHARED. Holds the work list with nodes of the current lvl. Database thread and master thread have access. */
		WorkList m_workList;
		
		/** m_firstDirty[ i ] contains the first dirty (i.e. not persisted) Morton in lvl i. */
		vector< MortonCode, ManagedAllocator< MortonCode > > m_firstDirty;
		
		OctreeDim m_octreeDim;
		
		string m_plyFilename;
		
		/** Mutex for the work list. */
		mutex m_listMutex;
		
		size_t m_memoryLimit;
		
		ulong m_expectedLoadPerThread;
		
		constexpr int M_N_THREADS;
	};
	
	template< typename Morton, typename Point >
	Node HierarchyCreator< Morton, Point >::
	createFromSortedFile( const size_t maxUsedMemory )
	{
		// SHARED. The disk access thread sets this true whenever it finishes reading all nodes from the current lvl.
		bool lvlFinished;
		
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
								m_releaseFlag.wait( lock, []{ return !isReleasing; } );
							
							points.push_back( makeManaged< Point >( p ) );
						}
						else 
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
						}
					}
				);
				
				lvlFinished = true;
				Direction direction = LEFT;
			}
		);
		
		uint lvl = m_octreeDim.m_leafLvl;
		
		// Hierarchy construction loop.
		while( lvl )
		{
			while( !lvlFinished )
			{
				if( m_workList.size() > 0 )
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
							input.pop_front();
							Morton parentCode = m_octreeDim.calcMorton( node ).getParent();
							
							vector< Node > siblings( 8 );
							siblings[ 0 ] = node;
							int nSiblings = 1;
							
							while( !input.empty() && m_octreeDim.calcMorton( input.front() ).getParent() == parentCode )
							{
								++nSiblings;
								siblings[ nSiblings ] = input.front();
								input.pop_front();
							}	
							
							if( nSiblings == 1 )
							{
								// Merging, just put the node to be processed in next level.
								output.push_front( siblings[ 0 ] );
							}
							else
							{
								// LOD
								Node inner = createInnerNode( siblings );
								output.push_front( inner );
							}
						}
					}

					for( int i = dispatchedThreads - 1; i > -1; --i )
					{
						mergeOrPushWork( iterOutput[ i - 1 ], iterOutput[ i ] );
					}
					mergeOrPushWork( m_nextLvlWorkList.front(), iterOutput[ 0 ] );
					
					// Check memory stress and release memory if necessary.
					if( AllocStatistics::totalAllocated() > maxUsedMemory )
					{
						isReleasing = true;
						releaseNodes( lvl );
						isReleasing = false;
						releaseFlag.notify_one();
					}
				}
			}
				
			--lvl;
			swapWorkLists();
		}
	}
	
	template< typename Morton, typename Point >
	inline void HierarchyCreator< Morton, Point >::releaseSiblings( Array< Node >& siblings, Sql& sql, uint nodeLvl )
	{
		for( int i = 0; i < siblings.size(); ++i )
		{
			if( !siblings[ i ].isLeaf() )
			{
				releaseSiblings( siblings[ i ].children(), sql, nodeLvl + 1 );
			}
		}
		
		// CONTINUE HERE. Calculating morton from m_octreeDim is wrong since the lvls are different.
		Morton firstSiblingMorton = m_octreeDim.calcMorton( siblings[ 0 ].getContents()[ 0 ] );
		Morton lastSiblingMorton = m_octreeDim.calcMorton( siblings[ siblings.size() - 1 ].getContents[ 0 ] );
		
		if( ( m_octreeDim.m_leafLvl - nodeLvl ) % 2 )
		{
			if( firstSiblingMorton )
		}
		else
		{
		}
		
		siblings.clear();
	}
	
	template< typename Morton, typename Point >
	inline void HierarchyCreator< Morton, Point >::releaseNodes( uint currentLvl )
	{
		auto workListIt = m_nextLvlWorkList.back();
		
		while( AllocStatistics::totalAllocated() > m_memoryLimit )
		{
			IterVector iterVector( M_N_THREADS );
			for( int i = 0; i < M_N_THREADS; ++i )
			{
				iterVector[ i ] = *( workListIt-- );
			}
			
			#pragma omp parallel for
			for( int i = 0; i < M_N_THREADS; ++i )
			{
				Sql& sql = m_dbs[ i ];
				sql.beginTransaction();
				
				NodeList& nodeList = iterVector[ i ];
				for( Node& node : nodeList )
				{
					if( !node.isLeaf() )
					{
						releaseSiblings( node.children(), sql, currentLvl );
					}
				}
				
				sql.endTransaction();
			}
		}
	}
	
	template< typename Morton, typename Point >
	inline Node HierarchyCreator< Morton, Point >::createInnerNode( vector< Node >&& vChildren ) const
	{
		using Map = map< int, Node&, less< int >, ManagedAllocator< pair< const int, Node& > > >;
		using MapEntry = typename Map::value_type;
		
		Array< Node > children( vChildren );
		
		int nPoints = 0;
		Map prefixMap();
		for( int i = 0; i < children.size(); ++i )
		{
			Node& child = children[ i ];
			prefixMap.insert( prefixMap.end(), MapEntry( nPoints, child ) );
			nPoints += child.getContents().size();
		}
		
		// LoD has 1/8 of children points.
		int numSamplePoints = std::max( 1., nPoints * 0.125 );
		Array< PointPtr > selectedPoints( numSamplePoints );
		
		for( int i = 0; i < numSamplePoints; ++i )
		{
			int choosenIdx = rand() % nPoints;
			MapEntry choosenEntry = --prefixMap.upper_bound( choosenIdx );
			selectedPoints[ i ] = choosenEntry.second.getContents[ choosenIdx - choosenEntry.first ];
		}
		
		return Node( selectedPoints, false );
	}
}

#endif