#ifndef OUT_OF_CORE_OCTREE_H
#define OUT_OF_CORE_OCTREE_H

#include "FrontOctree.h"
#include "PlyPointReader.h"
#include "SQLiteManager.h"

using namespace std;
using namespace util;

namespace model
{
	/** Octree for massive point clouds that cannot be held in main memory because of their sizes. The main memory is used
	 * as a cache, with data being fetched on demand from a database in disk. */
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point, typename Front,
			  typename FrontInsertionContainer >
	class OutOfCoreOctree
	: public FrontOctree< MortonPrecision, Float, Vec3, Point, Front, FrontInsertionContainer >
	{
		using MortonCode = model::MortonCode< MortonPrecision >;
		using MortonCodePtr = shared_ptr< MortonCode >;
		using PointPtr = shared_ptr< Point >;
		using PointVector = vector< PointPtr >;
		using OctreeNode = model::OctreeNode< MortonPrecision, Float, Vec3 >;
		using PlyPointReader = util::PlyPointReader< Float, Vec3, Point >;
		using SQLiteManager = util::SQLiteManager< Point, MortonCode, OctreeNode >;
		using ParentOctree = model::FrontOctree< MortonPrecision, Float, Vec3, Point, Front, FrontInsertionContainer >;
		
	public:
		OutOfCoreOctree( const int& maxPointsPerNode, const int& maxLevel );
		
		virtual void build( PointVector& points ) override;
		
		/** Builds octree using the database. */
		virtual void build();
		
		virtual void buildFromFile( const string& plyFileName, const typename PlyPointReader::Precision& precision,
									const Attributes& attribs ) override;
		
		SQLiteManager& getSQLiteManager() { return m_sqLite; }
	
	protected:
		virtual void buildBoundaries( const PointVector& points ) override;
		
		/** Build boundaries using the database. */
		virtual void buildBoundaries();
		
		virtual void buildNodes( PointVector& points ) override;
		
		virtual void buildLeaves( const PointVector& points ) override;
		
		virtual void insertPointInLeaf( const PointPtr& point, const MortonCodePtr& code );
		
		SQLiteManager m_sqLite;
	};
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point, typename Front,
			  typename FrontInsertionContainer >
	OutOfCoreOctree< MortonPrecision, Float, Vec3, Point, Front, FrontInsertionContainer >::
	OutOfCoreOctree( const int& maxPointsPerNode, const int& maxLevel )
	: ParentOctree( maxPointsPerNode, maxLevel )
	{}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point, typename Front,
			  typename FrontInsertionContainer >
	void OutOfCoreOctree< MortonPrecision, Float, Vec3, Point, Front, FrontInsertionContainer >
	::build( PointVector& points )
	{
		throw logic_error(  "build( PointVector& ) is unsuported. Use buildFromFile or another non out of core octree"
							"implementation" );
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point, typename Front,
			  typename FrontInsertionContainer >
	void OutOfCoreOctree< MortonPrecision, Float, Vec3, Point, Front, FrontInsertionContainer >
	::build()
	{
		buildBoundaries();
		//buildNodes();
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point, typename Front,
			  typename FrontInsertionContainer >
	void OutOfCoreOctree< MortonPrecision, Float, Vec3, Point, Front, FrontInsertionContainer >
	::buildFromFile( const string& plyFileName, const typename PlyPointReader::Precision& precision,
					 const Attributes& attribs )
	{
		// Octree boundary variables.
		Float negInf = -numeric_limits< Float >::max();
		Float posInf = numeric_limits< Float >::max();
		Vec3 minCoords( posInf, posInf, posInf );
		Vec3 maxCoords( negInf, negInf, negInf );
		
		// Whenever a point is read, update the boundary variables, and insert it into a leaf node.
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
			}
		);
		reader->read( plyFileName, precision, attribs );
		
		cout << "After reading points" << endl << endl;
		cout << "Attributes:" << reader->getAttributes() << endl << endl;
		
		// Save boundary info.
		*ParentOctree::m_origin = minCoords;
		*ParentOctree::m_size = maxCoords - minCoords;
		*ParentOctree::m_leafSize = *ParentOctree::m_size *
									( ( Float )1 / ( ( MortonPrecision )1 << ParentOctree::m_maxLevel ) );
		
		// From now on the reader is not necessary. Delete it in order to save memory.
		delete reader;
		
		//build();
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point, typename Front,
			  typename FrontInsertionContainer >
	void OutOfCoreOctree< MortonPrecision, Float, Vec3, Point, Front, FrontInsertionContainer >
	::buildBoundaries( const PointVector& points )
	{
		throw logic_error(  "buildBoundaries( PointVector& ) is unsuported. Use buildBoundaries() to take into consideration"
							" the database or another non out of core octree implementation" );
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point, typename Front,
			  typename FrontInsertionContainer >
	void OutOfCoreOctree< MortonPrecision, Float, Vec3, Point, Front, FrontInsertionContainer >
	::buildBoundaries()
	{
		
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point, typename Front,
			  typename FrontInsertionContainer >
	void OutOfCoreOctree< MortonPrecision, Float, Vec3, Point, Front, FrontInsertionContainer >
	::buildNodes( PointVector& points )
	{
		throw logic_error(  "buildNodes( PointVector& ) is unsuported. Use ***() to take into consideration"
							" the database or another non out of core octree implementation" );
	}
		
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point, typename Front,
			  typename FrontInsertionContainer >
	void OutOfCoreOctree< MortonPrecision, Float, Vec3, Point, Front, FrontInsertionContainer >
	::buildLeaves( const PointVector& points )
	{
		throw logic_error(  "buildLeaves( PointVector& ) is unsuported. Use ***() to take into consideration"
							" the database or another non out of core octree implementation" );
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point, typename Front,
			  typename FrontInsertionContainer >
	inline void OutOfCoreOctree< MortonPrecision, Float, Vec3, Point, Front, FrontInsertionContainer >
	::insertPointInLeaf( const PointPtr& point, const MortonCodePtr& code )
	{
		/*typename OctreeMap::iterator genericLeafIt = ParentOctree::m_hierarchy->find( code );
		
		unsigned long index = m_nPoints++;
		
		if( genericLeafIt == RandomSampleOctree::m_hierarchy->end() )
		{
			// Creates leaf node.
			IndexVector indices;
			indices.push_back( index );
			auto leafNode = make_shared< LeafNode >();
			leafNode->setContents( indices );
			( *RandomSampleOctree::m_hierarchy )[ code ] = leafNode;
		}
		else
		{
			// Node already exists. Appends the point there.
			OctreeNodePtr leafNode = genericLeafIt->second;
			shared_ptr< IndexVector > indices = leafNode-> template getContents< IndexVector >();
			indices->push_back( index );
		}*/
	}
	
	// ====================== Type Sugar ================================ /
	
	template< typename Float, typename Vec3, typename Point >
	using ShallowOutOfCoreOctree = OutOfCoreOctree< unsigned int, Float, Vec3, Point,
													unordered_set< MortonCode< unsigned int > >, vector< MortonCode< unsigned int > > >;
}

#endif