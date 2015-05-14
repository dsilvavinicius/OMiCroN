#ifndef OUT_OF_CORE_OCTREE_H
#define OUT_OF_CORE_OCTREE_H

#include <sqlite3.h>
#include <stdexcept>
#include <functional>
#include "IndexedOctree.h"
#include "SQLiteHelper.h"

using namespace std;
using namespace util;

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
		OutOfCoreOctree( const int& maxPointsPerNode, const int& maxLevel );
		~OutOfCoreOctree();
		
	private:
		
		sqlite3* m_db;
		sqlite3_stmt* m_nodeInsertion;
		sqlite3_stmt* m_pointQuery;
		sqlite3_stmt* m_nodeQuery;
		sqlite3_stmt* m_nodeIntervalQuery;
	};
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point  >
	OutOfCoreOctree< MortonPrecision, Float, Vec3, Point >::OutOfCoreOctree( const int& maxPointsPerNode,
																			 const int& maxLevel )
	: ParentOctree( maxPointsPerNode, maxLevel )
	{
		// Creating database, hierarchy and point tables.
		SQLiteHelper::safeCall(
			[ & ] ()
			{
				return sqlite3_open_v2( "Octree", &m_db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX,
										NULL );
				
			}
		);
		
		sqlite3_stmt* creationStmt;
		SQLiteHelper::safeCall(
			[ & ] ()
			{
			return sqlite3_prepare_v2( m_db,
										"CREATE TABLE Nodes ("
											"Morton INT NOT NULL PRIMARY KEY,"
											"Node BLOB"
										");\0",
										-1, &creationStmt, NULL
									);
			}
		);
		SQLiteHelper::safeCall( [ & ] () { return sqlite3_step( creationStmt ); } );
		SQLiteHelper::safeCall( [ & ] () { return sqlite3_finalize( creationStmt ); } );
		
		// Creating insertion statements.
		SQLiteHelper::safeCall(
			[ & ] ()
			{
				return sqlite3_prepare_v2( m_db, "INSERT INTO Nodes VALUES ( ?, ? )\0", -1, &m_nodeInsertion, NULL );
			}
		);
		
		// Creating queries.
		SQLiteHelper::safeCall(
			[ & ] ()
			{
				return sqlite3_prepare_v2( m_db, "SELECT Point FROM Points WHERE Id = ?\0", -1, &m_pointQuery, NULL );
			}
		);
		SQLiteHelper::safeCall(
			[ & ] ()
			{
				return sqlite3_prepare_v2( m_db, "SELECT Node FROM Nodes WHERE Morton = ?\0", -1, &m_nodeQuery, NULL );
			}
		);
		SQLiteHelper::safeCall(
			[ & ] ()
			{
				return sqlite3_prepare_v2( m_db, "SELECT Node FROM Nodes WHERE Morton BETWEEN ? AND ?\0", -1,
										   &m_nodeIntervalQuery, NULL );
			}
		);
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point  >
	OutOfCoreOctree< MortonPrecision, Float, Vec3, Point >::~OutOfCoreOctree()
	{
		sqlite3_finalize( m_pointQuery );
		sqlite3_finalize( m_nodeQuery );
		sqlite3_finalize( m_nodeIntervalQuery );
		sqlite3_close( m_db );
	}
	
	// ====================== Type Sugar ================================ /
	
	template< typename Float, typename Vec3, typename Point >
	using ShallowOutOfCoreOctree = IndexedOctree< unsigned int, Float, Vec3, Point >;
}

#endif