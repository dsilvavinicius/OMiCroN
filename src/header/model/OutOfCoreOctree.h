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
		using IndexVector = vector< unsigned int >;
		using OctreeNode = model::OctreeNode< MortonCode >;
		using OctreeNodePtr = shared_ptr< OctreeNode >;
		using OctreeMap = model::OctreeMap< MortonCode >;
		using OctreeMapPtr = shared_ptr< OctreeMap >;
		using PlyPointReader = util::PlyPointReader< Point >;
		using SQLiteManager = util::SQLiteManager< Point, MortonCode, OctreeNode >;
		using ParentOctree = model::FrontOctree< MortonCode, Point, Front, FrontInsertionContainer >;
		
	public:
		OutOfCoreOctree( const int& maxPointsPerNode, const int& maxLevel );
		
		/** Builds octree using the database. */
		virtual void build();
		
		virtual void buildFromFile( const string& plyFileName, const typename PlyPointReader::Precision& precision,
									const Attributes& attribs ) override;
		
		SQLiteManager& getSQLiteManager() { return m_sqLite; }
		
		/** DEPRECATED: use build() instead. */
		virtual void build( PointVector& points ) override;
		
		/** DEPRECATED. */
		PointVector& getPoints() override;
	
	protected:
		virtual void insertPointInLeaf( const PointPtr& point ) override;
		
		/** Acquires a node with given morton code. Searches in-memory hierarchy first and database if not found.
		 *	@return A smart-pointer to the node or nullptr if not found. */
		OctreeNodePtr getNode( const MortonCodePtr& code );
		
		/** DEPRECATED: use buildBoundaries() instead. */
		virtual void buildBoundaries( const PointVector& points ) override;
		
		/** DEPRECATED. */
		virtual void buildNodes( PointVector& points ) override;
		
		/** DEPRECATED. */
		virtual void buildLeaves( const PointVector& points ) override;
		
	private:
		/** Release memory by persisting nodes in the in-memory hierarchy. */
		void persistAndReleaseNodes();
		
		SQLiteManager m_sqLite;
		
		//unordered_map< unsigned long > m_points;
		
		static unsigned long M_MAX_MEMORY_BYTES;
		static double M_MIN_FREE_MEMORY_BYTES;
		static unsigned int M_NODES_PER_PERSISTENCE_ITERATION;
		static unsigned int M_POINTS_CREATED_UNTIL_PERSISTENCE;
	};
	
	template< typename MortonCode, typename Point, typename Front, typename FrontInsertionContainer >
	unsigned long OutOfCoreOctree< MortonCode, Point, Front, FrontInsertionContainer >
	::M_MAX_MEMORY_BYTES = MemoryInfo::getMemorySize();
	
	template< typename MortonCode, typename Point, typename Front, typename FrontInsertionContainer >
	double OutOfCoreOctree< MortonCode, Point, Front, FrontInsertionContainer >
	::M_MIN_FREE_MEMORY_BYTES = ( (double) M_MAX_MEMORY_BYTES * 0.1 );
	
	template< typename MortonCode, typename Point, typename Front, typename FrontInsertionContainer >
	unsigned int OutOfCoreOctree< MortonCode, Point, Front, FrontInsertionContainer >
	::M_NODES_PER_PERSISTENCE_ITERATION = 50;
	
	template< typename MortonCode, typename Point, typename Front, typename FrontInsertionContainer >
	unsigned int OutOfCoreOctree< MortonCode, Point, Front, FrontInsertionContainer >
	::M_POINTS_CREATED_UNTIL_PERSISTENCE = 100;
	
	template< typename MortonCode, typename Point, typename Front, typename FrontInsertionContainer >
	OutOfCoreOctree< MortonCode, Point, Front, FrontInsertionContainer >::OutOfCoreOctree( const int& maxPointsPerNode,
																						   const int& maxLevel )
	: ParentOctree( maxPointsPerNode, maxLevel )
	{}
	
	template< typename MortonCode, typename Point, typename Front, typename FrontInsertionContainer >
	void OutOfCoreOctree< MortonCode, Point, Front, FrontInsertionContainer >::build( PointVector& points )
	{
		throw logic_error(  "build( PointVector& ) is unsuported. Use buildFromFile or another non out of core octree"
							"implementation" );
	}
	
	template< typename MortonCode, typename Point, typename Front, typename FrontInsertionContainer >
	void OutOfCoreOctree< MortonCode, Point, Front, FrontInsertionContainer >::build()
	{
		//buildBoundaries();
		//buildNodes();
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
				
				//if( ParentOctree::m_nPoints )
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
		
		//build();
	}
	
	template< typename MortonCode, typename Point, typename Front, typename FrontInsertionContainer >
	inline void OutOfCoreOctree< MortonCode, Point, Front, FrontInsertionContainer >
	::insertPointInLeaf( const PointPtr& point )
	{
		MortonCodePtr code = make_shared< MortonCode >( ParentOctree::calcMorton( *point ) );
		OctreeNodePtr node = getNode( code );
		
		unsigned long index = ParentOctree::m_nPoints++;
		
		if( node == nullptr )
		{
			// Creates leaf node.
			IndexVector indices;
			indices.push_back( index );
			auto leafNode = make_shared< LeafNode< MortonCode, IndexVector > >();
			leafNode->setContents( indices );
			( *ParentOctree::m_hierarchy )[ code ] = leafNode;
		}
		else
		{
			// Node already exists. Appends the point there.
			shared_ptr< IndexVector > indices = node-> template getContents< IndexVector >();
			indices->push_back( index );
		}
		
		// START HERE FROM CONDITIONAL NODE PERSISTENCE!
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
			OctreeNode* node = m_sqLite. template getNode< IndexVector >( *code );
			if( node )
			{
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
		while( MemoryInfo::getAvailableMemorySize() < M_MIN_FREE_MEMORY_BYTES )
		{
			OctreeMapPtr hierarchy = ParentOctree::m_hierarchy;
			typename OctreeMapPtr::iterator nodeIt = hierarchy->rbegin();
			
			int i = 0;
			while( i < M_NODES_PER_PERSISTENCE_ITERATION && nodeIt != hierarchy->rend() )
			{
				MortonCodePtr code = nodeIt->first;
				OctreeNodePtr node = nodeIt->second;
				
				typename OctreeMapPtr::iterator toErase = nodeIt;
				
				++nodeIt;
				++i;
				
				hierarchy->erase( toErase );
				m_sqLite-> template insertNode< IndexVector >( *code, *node );
			}
		}
	}
	
	template< typename MortonCode, typename Point, typename Front, typename FrontInsertionContainer >
	vector< shared_ptr< Point > >& OutOfCoreOctree< MortonCode, Point, Front, FrontInsertionContainer >::getPoints()
	{
		throw logic_error( "getPoints() is unsuported, since everything is in database in this implementation." );
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