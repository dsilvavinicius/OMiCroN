#ifndef OUT_OF_CORE_OCTREE_H
#define OUT_OF_CORE_OCTREE_H

#include "Stream.h"
#include "FrontOctree.h"
#include "PlyPointReader.h"
#include "SQLiteManager.h"
#include "Profiler.h"

using namespace std;
using namespace util;

namespace model
{
	/** Octree for massive point clouds that cannot be held in main memory because of their sizes. The main memory is
	 * used as a cache, with data being fetched on demand from a database in disk. The persistence is tracked, so
	 * the construction and front tracking algorithms try to minimize database access. */
	template< typename OctreeParams, typename Front, typename FrontInsertionContainer >
	class OutOfCoreOctree
	: public FrontOctree< OctreeParams, Front, FrontInsertionContainer >
	{
	public:
		using MortonCode = typename OctreeParams::Morton;
		using MortonCodePtr = shared_ptr< MortonCode >;
		
		using Point = typename OctreeParams::Point;
		using PointPtr = shared_ptr< Point >;
		using PointVector = vector< PointPtr, ManagedAllocator< PointPtr > >;
		
		using OctreeNode = typename OctreeParams::Node;
		using OctreeNodePtr = shared_ptr< OctreeNode >;
		
		using OctreeMap = typename OctreeParams::Hierarchy;
		using OctreeMapPtr = shared_ptr< OctreeMap >;
		
		using ParentOctree = model::FrontOctree< OctreeParams, Front, FrontInsertionContainer >;
		using PlyPointReader = util::PlyPointReader< Point >;
		using SQLiteManager = util::SQLiteManager< Point, MortonCode, OctreeNode >;
		using IdNode = model::IdNode< MortonCode, OctreeNode >;
		using IdNodeVector = model::IdNodeVector< MortonCode, OctreeNode >;
		using SQLiteQuery = util::SQLiteQuery< IdNode >;
		
		/** Encapsulates memory management setup parameters. */
		struct MemorySetup
		{
			MemorySetup( const float& freeMemPercentThreshToStartRelease, const float& freeMemPercentThreshAfterRelease,
						 const uint& nNodesCreatedUntilPersistence, const uint& nSiblingGroupsPerLoad,
				const uint& maxNodeRequestsPerFrame )
			{
				if( freeMemPercentThreshToStartRelease <= 0.f || freeMemPercentThreshToStartRelease >= 1.f )
				{
					throw logic_error( "MemorySetup expects 0.f < freeMemPercentThreshToStartRelease < 1.f" );
				}
				if( freeMemPercentThreshAfterRelease <= 0.f || freeMemPercentThreshAfterRelease >= 1.f )
				{
					throw logic_error( "MemorySetup expects 0.f < freeMemPercentThreshAfterRelease < 1.f" );
				}
				if( freeMemPercentThreshAfterRelease < freeMemPercentThreshToStartRelease )
				{
					throw logic_error( "MemorySetup expects freeMemPercentThreshToStartRelease < "
										"freeMemPercentThreshAfterRelease < 1.f" );
				}
				
				m_freeMemPercentThreshToStartRelease = freeMemPercentThreshToStartRelease;
				m_freeMemPercentThreshAfterRelease = freeMemPercentThreshAfterRelease;
				m_nNodesCreatedUntilPersistence = nNodesCreatedUntilPersistence;
				m_nSiblingGroupsPerLoad = nSiblingGroupsPerLoad;
				m_maxNodeRequestsPerFrame = maxNodeRequestsPerFrame;
			}
			
			float m_freeMemPercentThreshToStartRelease; // Memory percentage threshold that triggers node release routine.
			float m_freeMemPercentThreshAfterRelease; // Memory percentage threshold that triggers node release routine end.
			uint m_nNodesCreatedUntilPersistence; // Number of created nodes until persistence routine is triggered.
			uint m_nSiblingGroupsPerLoad; // Number of sibling groups loaded per node loading iteration.
			uint m_maxNodeRequestsPerFrame; // Maximum number of node requests per frame in traversal.
		};
		
		OutOfCoreOctree( const int& maxPointsPerNode, const int& maxLevel, const string& dbFilename,
						 const MemorySetup& memSetup = MemorySetup( 0.1f, 0.2f, 200, 100, 1000 ) );
		
		/** Builds octree using the database. */
		virtual void build();
		
		virtual void buildFromFile( const string& plyFileName, const typename PlyPointReader::Precision& precision )
									override;
		
		SQLiteManager& getSQLiteManager() { return m_sqLite; }
		
		/** DEPRECATED: use build() instead. */
		virtual void build( PointVector& points ) override;
		
		template< typename P, typename F, typename FIC >
		friend ostream& operator<<( ostream& out, OutOfCoreOctree< P, F, FIC >& octree );
	
	protected:
		virtual void insertPointInLeaf( const PointPtr& point ) override;
		
		/** Acquires a node with given morton code. Searches in-memory hierarchy first and database if not found.
		 * DOES NOT TRACK PERSISTENCE, since the traversal and creation algorithms expect the atomic persistence
		 * operation to have a per-sibling basis. 
		 * @return A smart-pointer to the node or nullptr if not found. */
		OctreeNodePtr getNode( const MortonCodePtr& code );
		
		/** Acquires all child nodes of a given parent node code. Searches in-memory hierarchy first and database if
		 * not found. It assumes that the parameter is a code for an inner node. Using this method for a leaf node code
		 * will result in unnecessary accesses to database. TRACKS PERSISTENCE.
		 * @return A vector with all found nodes. */
		vector< OctreeNodePtr > getChildren( const MortonCodePtr& parent );
		
		/** Get a query for a range of nodes in closed interval [ a, b ]. */
		SQLiteQuery getRangeInDB( const MortonCodePtr& a, const MortonCodePtr& b );
		
		void buildInners() override;
		
		void eraseNodes( const typename OctreeMap::iterator& first, const typename OctreeMap::iterator& last ) override;
		
		/** This implementation makes a node load request if the iterator indicates that the node was not found in
		 * memory. */
		void onPrunningItAcquired( const typename OctreeMap::iterator& it, const MortonCodePtr& code ) override;
		
		/** This implementation makes a node load request if the iterator indicates that the node was not found in
		 * memory. */
		void onBranchingItAcquired( const typename OctreeMap::iterator& it, const MortonCodePtr& code ) override;
		
		/** Override to also check for completed node requests. */
		void onTraversalEnd() override;
		
		/** DEPRECATED. */
		virtual void buildBoundaries( const PointVector& points ) override;
		
		/** DEPRECATED. */
		virtual void buildNodes( PointVector& points ) override;
		
		/** DEPRECATED. */
		virtual void buildLeaves( const PointVector& points ) override;
		
	private:
		/** Releases nodes in the in-memory hierarchy at hierarchy creation time in order to ease memory pressure.
		 * Persists all released "dirty" nodes in the database. TRACKS PERSISTENCE.
		 * @returns the last released node code or nullptr in case of no release. */
		MortonCodePtr releaseNodesAtCreation();
		
		/** Releases nodes while tracking front in order to ease memory pressure. All not-front node are eligible to
		 * release and it is done until the memory pressure threshold is achieved. */
		void releaseNodesAtFrontTracking();
		
		/** Persists and release leaf nodes at hierarchy creation in order to ease memory pressure. Since leaf nodes
		 * are acessed in a random pattern and a loaded node is assured to be modified in the near future, this method
		 * just assumes all in-memory nodes dirty, sending them to the database whenever released. This trait also imposes
		 * NO PERSISTENCE TRACKING. */
		void persistAndReleaseLeaves();
		
		/** Persists all leaf nodes in order to start bottom-up inner nodes creation. Also load a few nodes to start
		 working. DOESN'T TRACK PERSISTENCE. */
		void persistAllLeaves();
		
		/** Persists all dirty nodes currently in memory while in inner nodes creation. TRACKS PERSISTENCE. */
		void persistAllDirty();
		
		/** Checks if a node is dirty and needs to be persisted before released. */
		bool isDirty( const MortonCodePtr& code ) const;
		
		/** Load nodes from database at hierarchy creation and revalidates the iterator. */
		void loadNodesAndValidateIter( const MortonCodePtr& nextFirstChildCode, const MortonCodePtr& lvlBoundary,
									   typename OctreeMap::iterator& firstChildIt );
		
		/** Load sibling groups in a query.
		 * @param query is the query with nodes to load.
		 * @returns The first loaded MortonCode or nullptr if the query returns no node. */
		shared_ptr< MortonCode > loadSiblingGroups( SQLiteQuery& query );
		
		SQLiteManager m_sqLite;
		
		// Octree creation related data:
		
		/** Last fully persisted node's morton code. Any morton code less than this one is dirty and should be written
		 * in database before released. */
		MortonCodePtr m_lastDirty;
		
		unsigned long m_nodesUntilLastPersistence;
		
		MemorySetup m_memSetup;
	};
	
	template< typename OctreeParams, typename Front, typename FrontInsertionContainer >
	OutOfCoreOctree< OctreeParams, Front, FrontInsertionContainer >
	::OutOfCoreOctree( const int& maxPointsPerNode, const int& maxLevel, const string& dbFilename,
					   const MemorySetup& memSetup )
	: ParentOctree( maxPointsPerNode, maxLevel ),
	m_nodesUntilLastPersistence( 0uL ),
	m_sqLite( dbFilename ),
	m_lastDirty( makeManaged< MortonCode >( MortonCode::getLvlLast( maxLevel ) ) ),
	m_memSetup( memSetup )
	{}
	
	template< typename OctreeParams, typename Front, typename FrontInsertionContainer >
	void OutOfCoreOctree< OctreeParams, Front, FrontInsertionContainer >::build()
	{
		buildInners();
	}
	
	template< typename OctreeParams, typename Front, typename FrontInsertionContainer >
	void OutOfCoreOctree< OctreeParams, Front, FrontInsertionContainer >
	::buildFromFile( const string& plyFileName, const typename PlyPointReader::Precision& precision )
	{
		auto start = Profiler::now();
		
		// Octree boundary variables.
		Float negInf = -numeric_limits< Float >::max();
		Float posInf = numeric_limits< Float >::max();
		Vec3 minCoords( posInf, posInf, posInf );
		Vec3 maxCoords( negInf, negInf, negInf );
		
		// The points are read in two passes. First to calculate octree boundaries and second to populate leaf nodes.
		// First pass: whenever a point is full read, update the boundary variables.
		auto *reader = new PlyPointReader( plyFileName );
		
		cout << "===== Starting first .ply file reading for octree boundaries calculation =====" << endl << endl;
		
		reader->read( precision,
					  [ & ]( const Point& point )
						{
							const Vec3& pos = point.getPos();
							
							for( int i = 0; i < 3; ++i )
							{
								minCoords[ i ] = glm::min( minCoords[ i ], pos[ i ] );
								maxCoords[ i ] = glm::max( maxCoords[ i ], pos[ i ] );
							}
						}
				);
		
		// Save boundary info.
		*ParentOctree::m_origin = minCoords;
		*ParentOctree::m_size = maxCoords - minCoords;
		*ParentOctree::m_leafSize = *ParentOctree::m_size *
									( ( Float )1 / ( ( unsigned long long )1 << ParentOctree::m_maxLevel ) );
		
		// Second pass: whenever a point is full read, inserts it in the hierarchy.
		*reader = PlyPointReader( plyFileName );
		
		cout << "===== Starting second .ply file reading for octree point insertion =====" << endl << endl;
		
		reader->read( precision,
			[ & ]( const Point& point )
			{
				insertPointInLeaf( makeManaged< Point >( point ) );
			}
		);
		
		// From now on the reader is not necessary. Delete it in order to save memory.
		delete reader;
		
		// Persist all leaves in order to start bottom-up octree creation.
		persistAllLeaves();
		
		cout << "Leaf construction time (ms):" << Profiler::elapsedTime( start ) << endl << endl;
		
		build();
		
		cout << "Total Hierarchy construction time (ms):" << Profiler::elapsedTime( start ) << endl << endl;
	}
	
	template< typename OctreeParams, typename Front, typename FrontInsertionContainer >
	inline void OutOfCoreOctree< OctreeParams, Front, FrontInsertionContainer >
	::insertPointInLeaf( const PointPtr& point )
	{
		MortonCodePtr code = makeManaged< MortonCode >( ParentOctree::calcMorton( *point ) );
			
		OctreeNodePtr node = getNode( code );
		
		if( node == nullptr )
		{
			++m_nodesUntilLastPersistence;
			
			// Creates leaf node.
			PointVector points;
			points.push_back( point );
			OctreeNodePtr leafNode = makeManaged< OctreeNode >( true );
			leafNode->setContents( points );
			( *ParentOctree::m_hierarchy )[ code ] = leafNode;
		}
		else
		{
			// Node already exists. Appends the point there.
			PointVector& points = node->getContents();
			points.push_back( point );
		}
		
		if( m_nodesUntilLastPersistence > m_memSetup.m_nNodesCreatedUntilPersistence )
		{
			persistAndReleaseLeaves();
		}
	}
	
	template< typename OctreeParams, typename Front, typename FrontInsertionContainer >
	inline shared_ptr< typename OctreeParams::Node > OutOfCoreOctree< OctreeParams, Front, FrontInsertionContainer >
	::getNode( const MortonCodePtr& code )
	{
		OctreeMapPtr hierarchy = ParentOctree::m_hierarchy;
		typename OctreeMap::iterator nodeIt = hierarchy->find( code );
		
		if( nodeIt != hierarchy->end() )
		{
			return nodeIt->second;
		}
		else
		{
			OctreeNodePtr node = m_sqLite.getNode( *code );
			( *hierarchy )[ code ] = node;
			if( node )
			{
				++m_nodesUntilLastPersistence;
				return node;
			}
			else
			{
				return nullptr;
			}
		}
	}
	
	template< typename OctreeParams, typename Front, typename FrontInsertionContainer >
	inline vector< shared_ptr< typename OctreeParams::Node > > OutOfCoreOctree
	< OctreeParams, Front, FrontInsertionContainer >
	::getChildren( const MortonCodePtr& parent )
	{
		MortonCodePtr firstChildCode = parent->getFirstChild();
		OctreeMapPtr hierarchy = ParentOctree::m_hierarchy;
		typename OctreeMap::iterator it = hierarchy->lower_bound( firstChildCode );
		typename OctreeMap::iterator end = hierarchy->end();
		vector< OctreeNodePtr > nodes;
		
		while( it != end && it->second->isChildOf( parent ) )
		{
			nodes.push_back( it->second );
			++it;
		}
		
		if( nodes.empty() )
		{
			// Nodes aren't in memory
			vector< IdNode > queried = m_sqLite. template getIdNodes< PointVector >( firstChildCode,
																					 parent->getLastChild() );
			for( IdNode idNode : queried )
			{
				MortonCodePtr code( idNode.first );
				OctreeNodePtr node( idNode.second );
				hierarchy[ code ] = node;
				
				nodes.push_back( node );
			}
		}
	}
	
	template< typename OctreeParams, typename Front, typename FrontInsertionContainer >
	inline SQLiteQuery< IdNode< typename OctreeParams::Morton, typename OctreeParams::Node > >
	OutOfCoreOctree< OctreeParams, Front, FrontInsertionContainer >
	::getRangeInDB(  const MortonCodePtr& a, const MortonCodePtr& b  )
	{
		return m_sqLite.getIdNodesQuery( *a, *b );
	}
	
	template< typename OctreeParams, typename Front, typename FrontInsertionContainer >
	inline void OutOfCoreOctree< OctreeParams, Front, FrontInsertionContainer >::persistAndReleaseLeaves()
	{
		IMemoryManager& memManager = SingletonMemoryManager::instance();
		if( !memManager.hasEnoughMemory( m_memSetup.m_freeMemPercentThreshToStartRelease ) )
		{
			// Debug
			cout << "Manager before release: " << endl << memManager << endl;
			//
			
			m_nodesUntilLastPersistence = 0;
			
			while( !memManager.hasEnoughMemory( m_memSetup.m_freeMemPercentThreshAfterRelease ) )
			{
				OctreeMapPtr hierarchy = ParentOctree::m_hierarchy;
				typename OctreeMap::reverse_iterator nodeIt = hierarchy->rbegin();
				
				m_sqLite.beginTransaction();
				
				int i = 0;
				while( i < m_memSetup.m_nNodesCreatedUntilPersistence )
				{
					MortonCodePtr code = nodeIt->first;
					OctreeNodePtr node = nodeIt->second;
					
					m_sqLite.insertNode( *code, *node );
					
					++i;
					nodeIt = typename OctreeMap::reverse_iterator( hierarchy->erase( std::next( nodeIt ).base() ) );
					
					if( nodeIt == hierarchy->rend() )
					{
						throw runtime_error( "Node release emptied hierarchy. This is not supposed to do." );
					}
				}
				
				m_sqLite.endTransaction();
			}
			
			// Debug
			cout << "Manager after release: " << endl << memManager << endl;
			//
		}
	}
	
	template< typename OctreeParams, typename Front, typename FrontInsertionContainer >
	void OutOfCoreOctree< OctreeParams, Front, FrontInsertionContainer >::persistAllLeaves()
	{
		cout << "===== Persisting all leaves before inner node creation =====" << endl << endl;
		
		OctreeMapPtr hierarchy = ParentOctree::m_hierarchy;
		
		m_sqLite.beginTransaction();
		
		// Send all in-memory leaves to database.
		for( auto elem : *hierarchy )
		{
			m_sqLite.insertNode( *elem.first, *elem.second );
		}
		
		m_sqLite.endTransaction();
		
		cout << "===== Ending leaf persistence =====" << endl << endl;
	}
	
	template< typename OctreeParams, typename Front, typename FrontInsertionContainer >
	void OutOfCoreOctree< OctreeParams, Front, FrontInsertionContainer >::persistAllDirty()
	{
		OctreeMapPtr hierarchy = ParentOctree::m_hierarchy;
		if( !hierarchy->empty() )
		{
			MortonCodePtr currentCode = nullptr;
			MortonCodePtr firstCode = hierarchy->begin()->first;
			
			if( isDirty( firstCode ) )
			{
				m_sqLite.beginTransaction();
				
				for( auto elem : *hierarchy )
				{
					currentCode = elem.first;
					
					if( !isDirty( currentCode ) )
					{
						break;
					}
					
					m_sqLite.insertNode( *currentCode, *elem.second );
				}
				
				m_sqLite.endTransaction();
				
				m_lastDirty = firstCode;
			}
		}
	}
	
	template< typename OctreeParams, typename Front, typename FrontInsertionContainer >
	inline shared_ptr< typename OctreeParams::Morton > OutOfCoreOctree
	< OctreeParams, Front, FrontInsertionContainer >
	::releaseNodesAtCreation()
	{
		OctreeMapPtr hierarchy = ParentOctree::m_hierarchy;
		typename OctreeMap::reverse_iterator nodeIt = hierarchy->rbegin();
		MortonCodePtr currentCode = nullptr;
		IMemoryManager& memManager = SingletonMemoryManager::instance();
		
		if( !memManager.hasEnoughMemory( m_memSetup.m_freeMemPercentThreshToStartRelease ) )
		{
			cout << "Manager before release: " << endl << memManager << endl;
			
			m_sqLite.beginTransaction();
			
			currentCode = nodeIt->first;
			
			while( !memManager.hasEnoughMemory( m_memSetup.m_freeMemPercentThreshAfterRelease ) &&
				nodeIt != hierarchy->rend() )
			{
				MortonCodePtr parentCode = currentCode->traverseUp();
				
				while( currentCode->isChildOf( *parentCode ) )
				{
					if( isDirty( currentCode ) )
					{
						m_sqLite.insertNode( *currentCode, *nodeIt->second );
					}
					
					// Little hack because reverse_iterator has an offset of 1 of its base (God knows why...).
					nodeIt = typename OctreeMap::reverse_iterator( hierarchy->erase( std::next( nodeIt ).base() ) );
					
					if( nodeIt == hierarchy->rend() )
					{
						currentCode = currentCode->getPrevious();
						break;
					}
					
					currentCode = nodeIt->first;
				}
			}
			
			if( isDirty( currentCode ) )
			{
				*m_lastDirty = *currentCode;
			}
			
			m_sqLite.endTransaction();
			
			cout << "Manager after release: " << endl << memManager << endl;
		}
		
		return currentCode;
	}
	
	template< typename OctreeParams, typename Front, typename FrontInsertionContainer >
	void OutOfCoreOctree< OctreeParams, Front, FrontInsertionContainer >::releaseNodesAtFrontTracking()
	{
		IMemoryManager& memManager = SingletonMemoryManager::instance();
		if( !memManager.hasEnoughMemory( m_memSetup.m_freeMemPercentThreshToStartRelease ) )
		{
			cout << "====== releaseNodesAtFrontTracking:Node release triggered ======" << endl << endl
				 << memManager << endl;
			
			OctreeMapPtr hierarchy = ParentOctree::m_hierarchy;
			
			for( auto it = ++hierarchy->begin(); // Skips root node
				it != hierarchy->end() && !memManager.hasEnoughMemory( m_memSetup.m_freeMemPercentThreshAfterRelease ); )
			{
				// Loops per siblings in order to avoid holes in hierarchy.
				auto current = it;
				MortonCodePtr parent = current->first->traverseUp();
				
				int nSiblings = 0;
				bool hasSiblingInFront = false;
				
				while( current != hierarchy->end() && current->first->isChildOf( *parent ) )
				{
					if( ParentOctree::m_frontBehavior->contains( *current->first ) )
					{
						hasSiblingInFront = true;
						break;
					}
					++nSiblings;
					++current;
				}
				
				if( !hasSiblingInFront )
				{
					auto postLastSibling = it;
					advance( postLastSibling, nSiblings );
					it = hierarchy->erase( it, postLastSibling );
				}
			}
			
			cout << "====== Node release ended ======" << endl << endl
				 << memManager << endl;
		}
	}
	
	// TODO: Changing the starting point of level construction iteration to the last nodes of the level imediately below
	// should make less nodes dirty when reseting m_lastDirty.
	template< typename OctreeParams, typename Front, typename FrontInsertionContainer >
	void OutOfCoreOctree< OctreeParams, Front, FrontInsertionContainer >::buildInners()
	{
		OctreeMapPtr hierarchy = ParentOctree::m_hierarchy;
		
		// Do a bottom-up per-level construction of inner nodes.
		for( int level = ParentOctree::m_maxLevel - 1; level > -1; --level )
		{
			cout << "========== Octree construction, level " << level << " ==========" << endl << endl;
			
			auto start = Profiler::now();
			
			// The idea behind this boundary is to get the minimum morton code that is from one level deeper than
			// the current one.
			MortonCodePtr lvlBoundary = makeManaged< MortonCode >( MortonCode::getLvlLast( level + 1 ) );
			
			//cout << "Acquiring nodes for current lvl." << endl << endl;
			{
				// Query nodes to load.
				SQLiteQuery query = getRangeInDB( makeManaged< MortonCode >( MortonCode::getLvlFirst( level + 1 ) ),
												  lvlBoundary );
			
				// Clear up hierarchy to ensure no holes.
				hierarchy->clear();
				
				// Get a few sibling groups in order to start next lvl creation.
				loadSiblingGroups( query );
				*m_lastDirty = MortonCode::getLvlLast( level );
			}
			
			typename OctreeMap::iterator firstChildIt = hierarchy->begin(); 
			typename OctreeMap::iterator hierarchyEnd = hierarchy->end();
			bool isLevelEnded = false;
			
			// Loops per siblings in a level.
			while( !isLevelEnded )
			{
				MortonCodePtr parentCode = firstChildIt->first->traverseUp();
				
				{
					auto children = vector< OctreeNodePtr >();
					
					// Puts children into vector.
					children.push_back( firstChildIt->second );
					
					typename OctreeMap::iterator currentChildIt = firstChildIt;
					while( ( ++currentChildIt ) != hierarchyEnd && *currentChildIt->first->traverseUp() == *parentCode )
					{
						OctreeNodePtr currentChild = currentChildIt->second;
						children.push_back( currentChild );
					}
					
					ParentOctree::buildInnerNode( firstChildIt, currentChildIt, parentCode, children );
					
					//cout << "After inner creation" << endl << endl;
					
					if( !isDirty( parentCode ) )
					{
						*m_lastDirty = *parentCode;
					}
				}
				
				// Release node if memory pressure is high enough.
				MortonCodePtr lastReleased = releaseNodesAtCreation();
				
				if( lastReleased == nullptr )
				{
					if( firstChildIt == hierarchyEnd )
					{
						MortonCodePtr nextFirstChildCode = parentCode->getLastChild()->getNext();
						if( *nextFirstChildCode <= *lvlBoundary )
						{
							// No more in-memory nodes in this lvl. Load more if any.
							loadNodesAndValidateIter( nextFirstChildCode, lvlBoundary, firstChildIt );
						}
					}
				}
				else
				{
					MortonCodePtr nextFirstChildCode = parentCode->getLastChild()->getNext();
					if( *lastReleased < *nextFirstChildCode )
					{
						// Nodes should be loaded and iterator revalidated, since it was released.
						loadNodesAndValidateIter( nextFirstChildCode, lvlBoundary, firstChildIt );
					}
				}
				
				if( firstChildIt == hierarchyEnd )
				{
					isLevelEnded = true;
				}
				else
				{
					isLevelEnded = !( *firstChildIt->first <= *lvlBoundary );
				}
			}
			
			//cout << "Persisting all dirty after at lvl ending." << endl << endl;
			
			// TODO: Instead of persisting all dirty nodes, I could persist only the nodes dirty in the current lvl.
			// Persists all nodes in left dirty in this lvl before proceeding one lvl up.
			persistAllDirty();
			
			cout << "========== End of level " << level << ". Time spent (ms): "
				 << Profiler::elapsedTime( start ) << "==========" << endl << endl;
		}
	}
	
	template< typename OctreeParams, typename Front, typename FrontInsertionContainer >
	inline void OutOfCoreOctree< OctreeParams, Front, FrontInsertionContainer >
	::loadNodesAndValidateIter( const MortonCodePtr& nextFirstChildCode, const MortonCodePtr& lvlBoundary,
								typename OctreeMap::iterator& firstChildIt )
	{
		OctreeMapPtr hierarchy = ParentOctree::m_hierarchy;
		typename OctreeMap::iterator hierarchyEnd = hierarchy->end();
		
		if( *lvlBoundary < *nextFirstChildCode )
		{
			// Next first child is from the lvl below, so the current lvl is ended. End iteration.
			firstChildIt = hierarchyEnd;
			return;
		}
		
		SQLiteQuery query = getRangeInDB( nextFirstChildCode, lvlBoundary );
		MortonCodePtr firstLoadedCode = loadSiblingGroups( query );
		
		if( firstLoadedCode == nullptr )
		{
			//No more nodes in this lvl. Nodes are not needed to be loaded. End iteration.
			firstChildIt = hierarchyEnd;
			return;
		}
		
		// Revalidating iterator
		firstChildIt = hierarchy->find( firstLoadedCode );
	}
	
	template< typename OctreeParams, typename Front, typename FrontInsertionContainer >
	shared_ptr< typename OctreeParams::Morton > OutOfCoreOctree< OctreeParams, Front, FrontInsertionContainer >
	::loadSiblingGroups( SQLiteQuery& query )
	{
		IdNode idNode;
		bool isQueryEnded = !query.step( &idNode );
		
		if( isQueryEnded )
		{
			return nullptr;
		}
		
		//cout << "Creating code to save the first loaded code" << endl;
		MortonCodePtr firstLoadedCode = makeManaged< MortonCode >( *idNode.first );
		MortonCodePtr currentCode( idNode.first );
		MortonCodePtr parentCode = currentCode->traverseUp();
		
		int nLoadedSiblingGroups = 0;
		for( int i = 0; i < m_memSetup.m_nSiblingGroupsPerLoad; ++i )
		{
			while( currentCode->isChildOf( *parentCode ) )
			{
				( *ParentOctree::m_hierarchy )[ currentCode ] = OctreeNodePtr( idNode.second );
				
				isQueryEnded = !query.step( &idNode );
				if( isQueryEnded )
				{
					break;
				}
				currentCode = MortonCodePtr( idNode.first );
			}
			
			if( isQueryEnded )
			{
				break;
			}
			
			parentCode = currentCode->traverseUp();
		}
		
		return firstLoadedCode;
	}
	
	// TODO: Find a way to let deletions also be in transactions.
	template< typename OctreeParams, typename Front, typename FrontInsertionContainer >
	inline void OutOfCoreOctree< OctreeParams, Front, FrontInsertionContainer >
	::eraseNodes( const typename OctreeMap::iterator& first, const typename OctreeMap::iterator& last )
	{
		auto prevLast = last;
		--prevLast;
		
		m_sqLite.deleteNodes( *first->first, *prevLast->first );
		RandomSampleOctree< OctreeParams >::eraseNodes( first, last );
	}
	
	template< typename OctreeParams, typename Front, typename FrontInsertionContainer >
	inline bool OutOfCoreOctree< OctreeParams, Front, FrontInsertionContainer >
	::isDirty( const MortonCodePtr& code ) const
	{
		return *code <= *m_lastDirty;
	}
	
	template< typename OctreeParams, typename Front, typename FrontInsertionContainer >
	inline void OutOfCoreOctree< OctreeParams, Front, FrontInsertionContainer >
	::onPrunningItAcquired( const typename OctreeMap::iterator& it, const MortonCodePtr& code )
	{
		onBranchingItAcquired( it, code ); // Both cases do the same thing: load the sibling groups related with code.
	}
	
	template< typename OctreeParams, typename Front, typename FrontInsertionContainer >
	inline void OutOfCoreOctree< OctreeParams, Front, FrontInsertionContainer >
	::onBranchingItAcquired( const typename OctreeMap::iterator& it, const MortonCodePtr& code )
	{
		if( it == ParentOctree::m_hierarchy->end() )
		{
			m_sqLite.requestNodesAsync( MortonInterval< MortonCode >( code, code->traverseUp()->getLastChild() ) );
		}
	}
	
	template< typename OctreeParams, typename Front, typename FrontInsertionContainer >
	void OutOfCoreOctree< OctreeParams, Front, FrontInsertionContainer >::onTraversalEnd()
	{
		ParentOctree::onTraversalEnd();
		
		releaseNodesAtFrontTracking();
		
		// Add requested nodes to hierarchy.
		vector< IdNodeVector > queries = m_sqLite.getRequestResults( m_memSetup.m_maxNodeRequestsPerFrame );
		OctreeMapPtr hierarchy = ParentOctree::m_hierarchy;
		
		for( IdNodeVector query : queries )
		{
			auto pastInsertionIt = hierarchy->upper_bound( query[ 0 ].first );
			
			for( IdNode idNode : query )
			{
				hierarchy->insert( pastInsertionIt, idNode );
			}
		}
	}
	
	template< typename OctreeParams, typename Front, typename FrontInsertionContainer >
	ostream& operator<<( ostream& out, OutOfCoreOctree< OctreeParams, Front, FrontInsertionContainer >& octree )
	{
		using PointPtr = shared_ptr< Point >;
		using PointVector = vector< PointPtr, ManagedAllocator< PointPtr > >;
		
		out << "====== OutOfCoreOctree ======" << endl << endl
			<< "Last Dirty: " << octree.m_lastDirty->getPathToRoot( true ) << endl
			<< octree.m_sqLite. template output< PointVector >() << endl
			<< SingletonMemoryManager::instance() << endl
			<< ( FrontOctree< OctreeParams, Front, FrontInsertionContainer >& ) octree << endl
			<< "====== End of OutOfCoreOctree ======" << endl;
		return out;
	}
	
	template< typename OctreeParams, typename Front, typename FrontInsertionContainer >
	void OutOfCoreOctree< OctreeParams, Front, FrontInsertionContainer >::buildBoundaries( const PointVector& points )
	{
		throw logic_error(  "buildBoundaries( PointVector& ) is unsuported. The boundaries are now calculated in"
							"buildFromFile()" );
	}
	
	template< typename OctreeParams, typename Front, typename FrontInsertionContainer >
	void OutOfCoreOctree< OctreeParams, Front, FrontInsertionContainer >::buildNodes( PointVector& points )
	{
		throw logic_error(  "buildNodes( PointVector& ) is unsuported. Use buildFromFile() to take into consideration"
							"the database or use FrontOctree." );
	}
		
	template< typename OctreeParams, typename Front, typename FrontInsertionContainer >
	void OutOfCoreOctree< OctreeParams, Front, FrontInsertionContainer >::buildLeaves( const PointVector& points )
	{
		throw logic_error(  "buildLeaves( PointVector& ) is unsuported. The leaves are now being built in "
							"buildFromFile()" );
	}
	
	template< typename OctreeParams, typename Front, typename FrontInsertionContainer >
	void OutOfCoreOctree< OctreeParams, Front, FrontInsertionContainer >::build( PointVector& points )
	{
		throw logic_error(  "build( PointVector& ) is unsuported. Use buildFromFile() instead." );
	}
	
	// ====================== Type Sugar ================================ /
	template< typename OctreeParams >
	using DefaultOutOfCoreOctree = OutOfCoreOctree< OctreeParams,
													unordered_set< typename OctreeParams::Morton >,
													vector< typename OctreeParams::Morton > >;
	
	DECLARE_OCTREE_TYPE(SPOpS,OutOfCoreOctree,DefaultOutOfCoreOctree,ShallowMortonCode,Point,OctreeNode< PointVector >,OctreeMap)
	
	DECLARE_OCTREE_TYPE(MPOpS,OutOfCoreOctree,DefaultOutOfCoreOctree,MediumMortonCode,Point,OctreeNode< PointVector >,OctreeMap)
	
	DECLARE_OCTREE_TYPE(SEOpS,OutOfCoreOctree,DefaultOutOfCoreOctree,ShallowMortonCode,ExtendedPoint,OctreeNode< ExtendedPointVector >,OctreeMap)
	
	DECLARE_OCTREE_TYPE(MEOpS,OutOfCoreOctree,DefaultOutOfCoreOctree,MediumMortonCode,ExtendedPoint,OctreeNode< ExtendedPointVector >,OctreeMap)
	
	DECLARE_OCTREE_TYPE(SPOiS,OutOfCoreOctree,DefaultOutOfCoreOctree,ShallowMortonCode,Point,OctreeNode< IndexVector >,OctreeMap)
	
	DECLARE_OCTREE_TYPE(MPOiS,OutOfCoreOctree,DefaultOutOfCoreOctree,MediumMortonCode,Point,OctreeNode< IndexVector >,OctreeMap)
	
	DECLARE_OCTREE_TYPE(SEOiS,OutOfCoreOctree,DefaultOutOfCoreOctree,ShallowMortonCode,ExtendedPoint,OctreeNode< IndexVector >,OctreeMap)
	
	DECLARE_OCTREE_TYPE(MEOiS,OutOfCoreOctree,DefaultOutOfCoreOctree,MediumMortonCode,ExtendedPoint,OctreeNode< IndexVector >,OctreeMap)
}

#endif