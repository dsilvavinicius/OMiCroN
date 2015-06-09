#ifndef OUT_OF_CORE_OCTREE_H
#define OUT_OF_CORE_OCTREE_H

#include "FrontOctree.h"
#include "PlyPointReader.h"
#include "SQLiteManager.h"
#include <MemoryInfo.h>

using namespace std;
using namespace util;

namespace model
{
	/** Octree for massive point clouds that cannot be held in main memory because of their sizes. The main memory is used
	 * as a cache, with data being fetched on demand from a database in disk. */
	template< typename MortonCode, typename Point, typename Front, typename FrontInsertionContainer >
	class OutOfCoreOctree
	: public FrontOctree< MortonCode, Point, Front, FrontInsertionContainer >
	{
		using MortonCodePtr = shared_ptr< MortonCode >;
		using PointPtr = shared_ptr< Point >;
		using PointVector = vector< PointPtr >;
		using OctreeNode = model::OctreeNode< MortonCode >;
		using OctreeNodePtr = shared_ptr< OctreeNode >;
		using OctreeMap = model::OctreeMap< MortonCode >;
		using OctreeMapPtr = shared_ptr< OctreeMap >;
		using PlyPointReader = util::PlyPointReader< Point >;
		using SQLiteManager = util::SQLiteManager< Point, MortonCode, OctreeNode >;
		using ParentOctree = model::FrontOctree< MortonCode, Point, Front, FrontInsertionContainer >;
		
	public:
		OutOfCoreOctree( const int& maxPointsPerNode, const int& maxLevel );
		~OutOfCoreOctree();
		
		/** Builds octree using the database. */
		virtual void build();
		
		virtual void buildFromFile( const string& plyFileName, const typename PlyPointReader::Precision& precision,
									const Attributes& attribs ) override;
		
		SQLiteManager& getSQLiteManager() { return m_sqLite; }
		
		/** DEPRECATED: use build() instead. */
		virtual void build( PointVector& points ) override;
	
	protected:
		virtual void insertPointInLeaf( const PointPtr& point ) override;
		
		/** Acquires a node with given morton code. Searches in-memory hierarchy first and database if not found.
		 * Also, it assumes that sequential getNode calls will have sequential ascendent morton code parameters, since it
		 * modifies m_lastInMemory if code is greater than the current m_lastInMemory value. In other words, the caller must
		 * ensure that all nodes with morton code less than the parameter are in-memory at call time.
		 *	@return A smart-pointer to the node or nullptr if not found. */
		OctreeNodePtr getNode( const MortonCodePtr& code );
		
		void buildInners() override;
		
		/** DEPRECATED: use buildBoundaries() instead. */
		virtual void buildBoundaries( const PointVector& points ) override;
		
		/** DEPRECATED. */
		virtual void buildNodes( PointVector& points ) override;
		
		/** DEPRECATED. */
		virtual void buildLeaves( const PointVector& points ) override;
		
	private:
		/** Releases memory in the in-memory hierarchy. Persists all "dirty" nodes in the database. */
		void releaseNodes();
		
		/** Persists and release leaf nodes. . */
		void persistAndReleaseLeaves();
		
		SQLiteManager m_sqLite;
		
		// Octree creation related data:
		
		/** Last fully persisted node's morton code. Any morton code less than this one is dirty and should be written in
		 * database before released. */
		MortonCode* m_lastPersisted;
		
		/** Last in-memory node's morton code. Any morton code greater than this one should be queried in database. */
		MortonCode* m_lastInMemory;
		
		unsigned long m_nodesUntilLastPersistence; 
		
		static unsigned int M_NODES_PER_PERSISTENCE_ITERATION;
		
		// End of octree creation related data.
		
		static unsigned long M_MAX_MEMORY_SIZE;
		static double M_MIN_FREE_MEMORY_SIZE;
	};
	
	template< typename MortonCode, typename Point, typename Front, typename FrontInsertionContainer >
	unsigned long OutOfCoreOctree< MortonCode, Point, Front, FrontInsertionContainer >
	::M_MAX_MEMORY_SIZE = MemoryInfo::getMemorySize();
	
	template< typename MortonCode, typename Point, typename Front, typename FrontInsertionContainer >
	double OutOfCoreOctree< MortonCode, Point, Front, FrontInsertionContainer >
	::M_MIN_FREE_MEMORY_SIZE = ( (double) M_MAX_MEMORY_SIZE * 0.1 );
	
	template< typename MortonCode, typename Point, typename Front, typename FrontInsertionContainer >
	unsigned int OutOfCoreOctree< MortonCode, Point, Front, FrontInsertionContainer >
	::M_NODES_PER_PERSISTENCE_ITERATION = 50;
	
	template< typename MortonCode, typename Point, typename Front, typename FrontInsertionContainer >
	OutOfCoreOctree< MortonCode, Point, Front, FrontInsertionContainer >::OutOfCoreOctree( const int& maxPointsPerNode,
																						   const int& maxLevel )
	: ParentOctree( maxPointsPerNode, maxLevel ),
	m_nodesUntilLastPersistence( 0uL )
	{
		lastPersisted = new MortonCode();
		lastPersisted->build( 7, 7, 7, maxLevel ); // Infinity.
		
		lastInMemory = new MortonCode();
		*lastInMemory = *lastPersisted; // Negative infinity.
	}
	
	template< typename MortonCode, typename Point, typename Front, typename FrontInsertionContainer >
	OutOfCoreOctree< MortonCode, Point, Front, FrontInsertionContainer >::~OutOfCoreOctree()
	{
		delete lastPersisted;
		delete lastInMemory;
	}
	
	template< typename MortonCode, typename Point, typename Front, typename FrontInsertionContainer >
	void OutOfCoreOctree< MortonCode, Point, Front, FrontInsertionContainer >::build( PointVector& points )
	{
		throw logic_error(  "build( PointVector& ) is unsuported. Use buildFromFile or another non out of core octree"
							"implementation" );
	}
	
	template< typename MortonCode, typename Point, typename Front, typename FrontInsertionContainer >
	void OutOfCoreOctree< MortonCode, Point, Front, FrontInsertionContainer >::build()
	{
		buildInners();
	}
	
	template< typename MortonCode, typename Point, typename Front, typename FrontInsertionContainer >
	void OutOfCoreOctree< MortonCode, Point, Front, FrontInsertionContainer >
	::buildFromFile( const string& plyFileName, const typename PlyPointReader::Precision& precision,
					 const Attributes& attribs )
	{
		// Octree boundary variables.
		Float negInf = -numeric_limits< Float >::max();
		Float posInf = numeric_limits< Float >::max();
		Vec3 minCoords( posInf, posInf, posInf );
		Vec3 maxCoords( negInf, negInf, negInf );
		
		// Whenever a point is full read, update the boundary variables, and insert it into a leaf node.
		auto *reader = new PlyPointReader(
			[ & ]( const Point& point )
			{
				const shared_ptr< const Vec3 > pos = point.getPos();
				
				for( int i = 0; i < 3; ++i )
				{
					minCoords[ i ] = glm::min( minCoords[ i ], ( *pos )[ i ] );
					maxCoords[ i ] = glm::max( maxCoords[ i ], ( *pos )[ i ] );
				}
				
				sqlite_int64 index = m_sqLite.insertPoint( point );
				
				insertPointInLeaf( make_shared< Point >( point ) );
			}
		);
		reader->read( plyFileName, precision, attribs );
		
		cout << "After reading points" << endl << endl;
		cout << "Attributes:" << reader->getAttributes() << endl << endl;
		
		// Save boundary info.
		*ParentOctree::m_origin = minCoords;
		*ParentOctree::m_size = maxCoords - minCoords;
		*ParentOctree::m_leafSize = *ParentOctree::m_size *
									( ( Float )1 / ( ( unsigned long long )1 << ParentOctree::m_maxLevel ) );
		
		// From now on the reader is not necessary. Delete it in order to save memory.
		delete reader;
		
		build();
	}
	
	template< typename MortonCode, typename Point, typename Front, typename FrontInsertionContainer >
	inline void OutOfCoreOctree< MortonCode, Point, Front, FrontInsertionContainer >
	::insertPointInLeaf( const PointPtr& point )
	{
		MortonCodePtr code = make_shared< MortonCode >( ParentOctree::calcMorton( *point ) );
		OctreeNodePtr node = getNode( code );
		
		if( node == nullptr )
		{
			++m_nodesUntilLastPersistence;
			
			// Creates leaf node.
			PointVector points;
			points.push_back( point );
			auto leafNode = make_shared< LeafNode< MortonCode, PointVector > >();
			leafNode->setContents( points );
			( *ParentOctree::m_hierarchy )[ code ] = leafNode;
		}
		else
		{
			// Node already exists. Appends the point there.
			shared_ptr< PointVector > points = node-> template getContents< PointVector >();
			points->push_back( point );
		}
		
		if( m_nodesUntilLastPersistence > M_NODES_PER_PERSISTENCE_ITERATION )
		{
			persistAndReleaseNodes();
		}
	}
	
	template< typename MortonCode, typename Point, typename Front, typename FrontInsertionContainer >
	inline OctreeNodePtr< MortonCode > OutOfCoreOctree< MortonCode, Point, Front, FrontInsertionContainer >
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
			OctreeNode* node = m_sqLite. template getNode< PointVector >( *code );
			if( node )
			{
				++m_nodesUntilLastPersistence;
				if( *code > *m_lastInMemory )
				{
					*m_lastInMemory = *code;
				}
				
				OctreeNodePtr nodePtr( node );
				return nodePtr;
			}
			else
			{
				return nullptr;
			}
		}
	}
	
	template< typename MortonCode, typename Point, typename Front, typename FrontInsertionContainer >
	inline void OutOfCoreOctree< MortonCode, Point, Front, FrontInsertionContainer >::persistAndReleaseNodes()
	{
		while( MemoryInfo::getAvailableMemorySize() < M_MIN_FREE_MEMORY_SIZE )
		{
			m_nodesUntilLastPersistence = 0;
			
			OctreeMapPtr hierarchy = ParentOctree::m_hierarchy;
			typename OctreeMap::reverse_iterator nodeIt = hierarchy->rbegin();
			
			int i = 0;
			while( i < M_NODES_PER_PERSISTENCE_ITERATION && nodeIt != hierarchy->rend() )
			{
				MortonCodePtr code = nodeIt->first;
				OctreeNodePtr node = nodeIt->second;
				
				typename OctreeMap::reverse_iterator toErase = nodeIt;
				
				++nodeIt;
				++i;
				
				hierarchy->erase( toErase.base() );
				m_sqLite. template insertNode< PointVector >( *code, *node );
			}
		}
	}
	
	template< typename MortonCode, typename Point, typename Front, typename FrontInsertionContainer >
	void OutOfCoreOctree< MortonCode, Point, Front, FrontInsertionContainer >::buildInners()
	{
		// Do a bottom-up per-level construction of inner nodes.
		for( int level = ParentOctree::m_maxLevel - 1; level > -1; --level )
		{
			cout << "========== Octree construction, level " << level << " ==========" << endl << endl;
			// The idea behind this boundary is to get the minimum morton code that is from lower levels than
			// the current. This is the same of the morton code filled with just one 1 bit from the level immediately
			// below the current one. 
			unsigned long long mortonLvlBoundary = ( unsigned long long )( 1 ) << ( 3 * ( level + 1 ) + 1 );
			
			//cout << "Morton lvl boundary: 0x" << hex << mortonLvlBoundary << dec << endl;
			
			typename OctreeMap::iterator firstChildIt = m_hierarchy->begin(); 
			
			// Loops per siblings in a level.
			while( firstChildIt != m_hierarchy->end() && firstChildIt->first->getBits() < mortonLvlBoundary )
			{
				MortonCodePtr parentCode = firstChildIt->first->traverseUp();
				
				auto children = vector< OctreeNodePtr >();
				
				// Puts children into vector.
				children.push_back( firstChildIt->second );
				
				typename OctreeMap::iterator currentChildIt = firstChildIt;
				while( ( ++currentChildIt ) != m_hierarchy->end() && *currentChildIt->first->traverseUp() == *parentCode )
				{
					OctreeNodePtr currentChild = currentChildIt->second;
					children.push_back( currentChild );
				}
				
				buildInnerNode( firstChildIt, currentChildIt, parentCode, children );
			}
			
			cout << "========== End of level " << level << " ==========" << endl << endl;
		}
	}
	
	template< typename MortonCode, typename Point, typename Front, typename FrontInsertionContainer >
	void OutOfCoreOctree< MortonCode, Point, Front, FrontInsertionContainer >::buildBoundaries( const PointVector& points )
	{
		throw logic_error(  "buildBoundaries( PointVector& ) is unsuported. Use buildBoundaries() to take into consideration"
							" the database or another non out of core octree implementation" );
	}
	
	template< typename MortonCode, typename Point, typename Front, typename FrontInsertionContainer >
	void OutOfCoreOctree< MortonCode, Point, Front, FrontInsertionContainer >::buildNodes( PointVector& points )
	{
		throw logic_error(  "buildNodes( PointVector& ) is unsuported. Use ***() to take into consideration"
							" the database or another non out of core octree implementation" );
	}
		
	template< typename MortonCode, typename Point, typename Front, typename FrontInsertionContainer >
	void OutOfCoreOctree< MortonCode, Point, Front, FrontInsertionContainer >::buildLeaves( const PointVector& points )
	{
		throw logic_error(  "buildLeaves( PointVector& ) is unsuported. Use ***() to take into consideration"
							" the database or another non out of core octree implementation" );
	}
	
	// ====================== Type Sugar ================================ /
	
	template< typename Point >
	using ShallowOutOfCoreOctree = OutOfCoreOctree	< ShallowMortonCode, Point, unordered_set< ShallowMortonCode >,
														vector< ShallowMortonCode >
													>;
}

#endif