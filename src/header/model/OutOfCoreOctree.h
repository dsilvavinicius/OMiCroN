#ifndef OUT_OF_CORE_OCTREE_H
#define OUT_OF_CORE_OCTREE_H

#include <sqlite3.h>
#include "IndexedOctree.h"

namespace model
{
	/** Octree for massive point clouds that cannot be held in main memory because of their sizes. The main memory is used
	 * as a cache, with data being fetched on demand from a database in disk. */
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point  >
	class OutOfCoreOctree
	: public IndexedOctree< MortonPrecision, Float, Vec3, Point >
	{
		using ParentOctree = model::IndexedOctree< MortonPrecision, Float, Vec3, Point >;
		
	public:
		OutOfCoreOctree();
		~OutOfCoreOctree();
		
	private:
		sqlite3* m_db;
		sqlite3_stmt m_pointQuery;
		sqlite3_stmt m_nodeQuery;
	};
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point  >
	OutOfCoreOctree< MortonPrecision, Float, Vec3, Point >::OutOfCoreOctree( const int& maxPointsPerNode,
																			 const int& maxLevel )
	: ParentOctree( maxPointsPerNode, maxLevel )
	{
		sqlite3_open_v2( "Octree", &m_db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL );
		
		sqlite3_stmt* creationStmt;
		sqlite3_prepare_v2( m_db, 	"CREATE TABLE Octree.Hierarchy ("
										"Morton INT,"
										"Node BLOB,"
										"PRIMARY KEY ( Morton )"
									").\0", -1, creationStmt, NULL );
		sqlite3_finalize( creationStmt );
		
		sqlite3_prepare_v2( m_db, "\0", -1, m_pointQuery, NULL );
		sqlite3_prepare_v2( m_db, "\0", -1, m_nodeQuery, NULL );
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point  >
	OutOfCoreOctree< MortonPrecision, Float, Vec3, Point >::~OutOfCoreOctree()
	{
		sqlite3_finalize( m_pointQuery );
		sqlite3_finalize( m_nodeQuery );
		sqlite3_close( m_db );
	}
}

#endif