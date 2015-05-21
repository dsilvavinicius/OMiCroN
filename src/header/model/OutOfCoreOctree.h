#ifndef OUT_OF_CORE_OCTREE_H
#define OUT_OF_CORE_OCTREE_H

#include "FrontOctree.h"
#include "OutOfCorePlyPointReader.h"
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
		using PlyPointReader = OutOfCorePlyPointReader< Float, Vec3, Point >;
		using SQLiteManager = util::SQLiteManager< Point >;
		using ParentOctree = model::FrontOctree< MortonPrecision, Float, Vec3, Point, Front, FrontInsertionContainer >;
		
	public:
		OutOfCoreOctree( const int& maxPointsPerNode, const int& maxLevel );
		
		void buildFromFile( const string& plyFileName, const typename PlyPointReader::Precision& precision, const Attributes& attribs ) override;
		
	private:
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
	::buildFromFile( const string& plyFileName, const typename PlyPointReader::Precision& precision, const Attributes& attribs )
	{
		auto *reader = new PlyPointReader( m_sqLite );
		reader->read( plyFileName, precision, attribs );
		
		cout << "After reading points" << endl << endl;
		cout << "Attributes:" << reader->getAttributes() << endl << endl;
		
		// From now on the reader is not necessary. Delete it in order to save memory.
		delete reader;
		
		//build( points );
	}
	
	// ====================== Type Sugar ================================ /
	
	template< typename Float, typename Vec3, typename Point >
	using ShallowOutOfCoreOctree = OutOfCoreOctree< unsigned int, Float, Vec3, Point,
													unordered_set< MortonCode< unsigned int > >, vector< MortonCode< unsigned int > > >;
}

#endif