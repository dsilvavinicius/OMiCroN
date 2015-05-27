#ifndef SQLITE_MANAGER_H
#define SQLITE_MANAGER_H

#include <stdexcept>
#include <functional>
#include <sqlite3.h>
#include <iostream>
#include <OctreeNode.h>

using namespace std;

namespace util
{
	/** Manages all SQLite operations. */
	template< typename Point, typename MortonCode, typename OctreeNode >
	class SQLiteManager
	{
	public:
		SQLiteManager();
		~SQLiteManager();
		
		/** Inserts point into database.
		 *	@returns the index of the pointer. The first index is 0 and each next point increments it by 1. */
		sqlite_int64 insertPoint( const Point& point );
		
		/** Searches the point in the database, given its index. */
		Point getPoint( const sqlite3_uint64& index );
		
		/** Inserts the node into database, using the given morton code as identifier.
		 *	@param NodeContents is the type of the contents of the node. */
		template< typename NodeContents >
		void insertNode( const MortonCode& morton, const OctreeNode& node );
		
		/** Searches the node in the database, given its morton code.
		 *	@param NodeContents is the type of the contents of the node.
		 *	@param morton is the node morton code id.
		 *	@returns the acquired node. The pointer ownership is caller's. */
		template< typename NodeContents >
		OctreeNode* getNode( const MortonCode& morton );
		
		/** Searches for a range of nodes in the database, given the morton code interval (a, b).
		 *	@param NodeContents is the type of the contents of the node.
		 *	@param a is the minor boundary of the morton code open interval.
		 *	@param b is the major boundary of the morton open interval.
		 *	@returns the acquired nodes. The ownership of the node pointers is caller's. */
		template< typename NodeContents >
		vector< OctreeNode* > getNodes( const MortonCode& a, const MortonCode& b );
		
	private:
		/** Release all acquired resources. */
		void release();
		
		/** Creates the needed tables in the database. */
		void createTables();
		
		/** Drops all needed tables. */
		void dropTables();
		
		/** Creates the needed statements for database operations. */
		void createStmts();
		
		/** Checks return code of a called SQLite function.*/
		void checkReturnCode( const int& returnCode, const int& expectedCode );
		
		/** Calls sqlite3_prepare_v2 safely. */
		void safePrepare( const char* stringStmt, sqlite3_stmt** stmt );
		
		/** Calls sqlite3_prepare_v2 unsafely. */
		void unsafePrepare( const char* stringStmt, sqlite3_stmt** stmt );
		
		/** Calls sqlite3_step function safely.
		 *  @returns true if a row was found, false otherwise.*/
		bool safeStep( sqlite3_stmt* statement );
		
		/** Calls sqlite3_step function unsafely. */
		void unsafeStep( sqlite3_stmt* statement );
		
		/** Safe finalizes an sqlite3_stmt. */
		void safeFinalize( sqlite3_stmt* statement );
		
		/** Safe finalizes an sqlite3_stmt. */
		void unsafeFinalize( sqlite3_stmt* statement );
		
		/** Resets a prepared sqlite3_stmt. */
		void safeReset( sqlite3_stmt* statement );
		
		sqlite3* m_db;
		
		sqlite3_stmt* m_pointInsertionStmt;
		sqlite3_stmt* m_pointQuery;
		
		sqlite3_stmt* m_nodeInsertion;
		sqlite3_stmt* m_nodeQuery;
		sqlite3_stmt* m_nodeIntervalQuery;
		
		/** Current number of inserted points. */
		sqlite_int64 m_nPoints;
	};
	
	template< typename Point, typename MortonCode, typename OctreeNode >
	SQLiteManager< Point, MortonCode, OctreeNode >::SQLiteManager()
	: m_db( nullptr ),
	m_pointInsertionStmt( nullptr ),
	m_pointQuery( nullptr ),
	m_nodeInsertion( nullptr ),
	m_nodeQuery( nullptr ),
	m_nodeIntervalQuery( nullptr ),
	m_nPoints( 0 )
	{
		checkReturnCode(
			sqlite3_open_v2( "Octree.db", &m_db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX, NULL ),
			SQLITE_OK
		);
		
		createTables();
		createStmts();
	}
	
	template< typename Point, typename MortonCode, typename OctreeNode >
	SQLiteManager< Point, MortonCode, OctreeNode >::~SQLiteManager()
	{
		release();
	}
	
	template< typename Point, typename MortonCode, typename OctreeNode >
	sqlite_int64 SQLiteManager< Point, MortonCode, OctreeNode >::insertPoint( const Point& point )
	{
		byte* serialization;
		size_t blobSize = point.serialize( &serialization );
		
		checkReturnCode( sqlite3_bind_int64( m_pointInsertionStmt, 1, m_nPoints ), SQLITE_OK );
		checkReturnCode( sqlite3_bind_blob( m_pointInsertionStmt, 2, serialization, blobSize, SQLITE_STATIC ), SQLITE_OK );
		safeStep( m_pointInsertionStmt );
		safeReset( m_pointInsertionStmt );
		
		delete[] serialization;
		
		return m_nPoints++;
	}
	
	template< typename Point, typename MortonCode, typename OctreeNode >
	Point SQLiteManager< Point, MortonCode, OctreeNode >::getPoint( const sqlite3_uint64& index )
	{
		checkReturnCode( sqlite3_bind_int64( m_pointQuery, 1, index ), SQLITE_OK );
		safeStep( m_pointQuery );
		
		byte* blob = ( byte* ) sqlite3_column_blob( m_pointQuery, 0 );
		Point point( blob );
		safeReset( m_pointQuery );
		
		return point;
	}
	
	template< typename Point, typename MortonCode, typename OctreeNode >
	template< typename NodeContents >
	void SQLiteManager< Point, MortonCode, OctreeNode >::insertNode( const MortonCode& morton, const OctreeNode& node )
	{
		byte* serialization;
		size_t blobSize = node. template serialize< NodeContents >( &serialization );
		
		checkReturnCode( sqlite3_bind_int64( m_nodeInsertion, 1, morton.getBits() ), SQLITE_OK );
		checkReturnCode( sqlite3_bind_blob( m_nodeInsertion, 2, serialization, blobSize, SQLITE_STATIC ), SQLITE_OK );
		
		safeStep( m_nodeInsertion );
		safeReset( m_nodeInsertion );
		
		delete[] serialization;
	}
	
	template< typename Point, typename MortonCode, typename OctreeNode >
	template< typename NodeContents >
	OctreeNode* SQLiteManager< Point, MortonCode, OctreeNode >::getNode( const MortonCode& morton )
	{
		checkReturnCode( sqlite3_bind_int64( m_nodeQuery, 1, morton.getBits() ), SQLITE_OK );
		safeStep( m_pointQuery );
		
		byte* blob = ( byte* ) sqlite3_column_blob( m_nodeQuery, 0 );
		OctreeNode* node = OctreeNode:: template deserialize< NodeContents >( blob );
		safeReset( m_pointQuery );
		
		return node;
	}
	
	template< typename Point, typename MortonCode, typename OctreeNode >
	template< typename NodeContents >
	vector< OctreeNode* > SQLiteManager< Point, MortonCode, OctreeNode >::getNodes( const MortonCode& a,
																					const MortonCode& b )
	{
		checkReturnCode( sqlite3_bind_int64( m_nodeIntervalQuery, 1, a.getBits() ), SQLITE_OK );
		checkReturnCode( sqlite3_bind_int64( m_nodeIntervalQuery, 2, b.getBits() ), SQLITE_OK );
		
		vector< OctreeNode* > nodes;
		
		while( safeStep( m_pointQuery ) )
		{
			byte* blob = ( byte* ) sqlite3_column_blob( m_nodeIntervalQuery, 0 );
			OctreeNode* node = OctreeNode:: template deserialize< NodeContents >( blob );
			nodes.push_back( node );
		}
		
		safeReset( m_pointQuery );
		
		return nodes;
	}
	
	template< typename Point, typename MortonCode, typename OctreeNode >
	void SQLiteManager< Point, MortonCode, OctreeNode >::createTables()
	{
		cout << "Creating tables." << endl;
		sqlite3_stmt* creationStmt;
		
		safePrepare(
			"CREATE TABLE IF NOT EXISTS Points ("
				"Id INTEGER PRIMARY KEY,"
				"Point BLOB"
			");",
			&creationStmt
		);
		safeStep( creationStmt );
		safeFinalize( creationStmt );
		
		safePrepare(
			"CREATE TABLE IF NOT EXISTS Nodes("
				"Morton INTEGER PRIMARY KEY,"
				"Node BLOB"
			");",
			&creationStmt
		);
		safeStep( creationStmt );
		safeFinalize( creationStmt );
		
		cout << "Ending Creating tables." << endl;
	}
	
	template< typename Point, typename MortonCode, typename OctreeNode >
	void SQLiteManager< Point, MortonCode, OctreeNode >::dropTables()
	{
		cout << "Dropping tables." << endl;
		sqlite3_stmt* dropStmt;
		
		unsafePrepare( "DROP TABLE IF EXISTS Points;", &dropStmt );
		unsafeStep( dropStmt );
		unsafeFinalize( dropStmt );
		
		unsafePrepare( "DROP TABLE IF EXISTS Nodes;", &dropStmt );
		unsafeStep( dropStmt );
		unsafeFinalize( dropStmt );
		cout << "Ending Dropping tables." << endl;
	}
	
	template< typename Point, typename MortonCode, typename OctreeNode >
	void SQLiteManager< Point, MortonCode, OctreeNode >::createStmts()
	{
		cout << "Creating stmts." << endl;
		safePrepare( "INSERT INTO Points VALUES ( ?, ? );", &m_pointInsertionStmt);
		safePrepare( "INSERT INTO Nodes VALUES ( ?, ? );", &m_nodeInsertion );
		safePrepare( "SELECT Point FROM Points WHERE Id = ?;", &m_pointQuery );
		safePrepare( "SELECT Node FROM Nodes WHERE Morton = ?;", &m_nodeQuery );
		safePrepare( "SELECT Node FROM Nodes WHERE Morton BETWEEN ? AND ?;", &m_nodeIntervalQuery );
		cout << "Ending Creating stmts." << endl;
	}
	
	template< typename Point, typename MortonCode, typename OctreeNode >
	void SQLiteManager< Point, MortonCode, OctreeNode >::release()
	{
		dropTables();
		
		unsafeFinalize( m_pointQuery );
		unsafeFinalize( m_nodeInsertion );
		unsafeFinalize( m_nodeQuery );
		unsafeFinalize( m_nodeIntervalQuery );
		
		if( m_db )
		{
			sqlite3_close( m_db );
		}
	}
	
	template< typename Point, typename MortonCode, typename OctreeNode >
	inline void SQLiteManager< Point, MortonCode, OctreeNode >::checkReturnCode( const int& returnCode,
																				 const int& expectedCode )
	{
		if( returnCode != expectedCode )
		{
			release();
			throw runtime_error( sqlite3_errstr( returnCode ) );
		}
	}
	
	template< typename Point, typename MortonCode, typename OctreeNode >
	inline void SQLiteManager< Point, MortonCode, OctreeNode >::safePrepare( const char* stringStmt, sqlite3_stmt** stmt )
	{
		checkReturnCode( sqlite3_prepare_v2( m_db, stringStmt, -1, stmt, NULL ), SQLITE_OK );
	}
	
	template< typename Point, typename MortonCode, typename OctreeNode >
	inline void SQLiteManager< Point, MortonCode, OctreeNode >::unsafePrepare( const char* stringStmt, sqlite3_stmt** stmt )
	{
		sqlite3_prepare_v2( m_db, stringStmt, -1, stmt, NULL );
	}
	
	template< typename Point, typename MortonCode, typename OctreeNode >
	inline bool SQLiteManager< Point, MortonCode, OctreeNode >::safeStep( sqlite3_stmt* statement )
	{
		int returnCode = sqlite3_step( statement );
		switch( returnCode )
		{
			case SQLITE_ROW: return true;
			case SQLITE_DONE: return false;
			default:
			{
				release();
				throw runtime_error( sqlite3_errstr( returnCode ) );
			}
		}
	}
	
	template< typename Point, typename MortonCode, typename OctreeNode >
	inline void SQLiteManager< Point, MortonCode, OctreeNode >::unsafeStep( sqlite3_stmt* statement )
	{
		sqlite3_step( statement );
	}
	
	template< typename Point, typename MortonCode, typename OctreeNode >
	inline void SQLiteManager< Point, MortonCode, OctreeNode >::safeFinalize( sqlite3_stmt* statement )
	{
		if( statement )
		{
			checkReturnCode( sqlite3_finalize( statement ), SQLITE_OK );
		}
	}
	
	template< typename Point, typename MortonCode, typename OctreeNode >
	inline void SQLiteManager< Point, MortonCode, OctreeNode >::unsafeFinalize( sqlite3_stmt* statement )
	{
		if( statement )
		{
			sqlite3_finalize( statement );
		}
	}
	
	template< typename Point, typename MortonCode, typename OctreeNode >
	inline void SQLiteManager< Point, MortonCode, OctreeNode >::safeReset( sqlite3_stmt* statement )
	{
		checkReturnCode( sqlite3_reset( statement ), SQLITE_OK );
	}
}

#endif