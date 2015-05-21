#ifndef SQLITE_MANAGER_H
#define SQLITE_MANAGER_H

#include <stdexcept>
#include <functional>
#include <sqlite3.h>
#include <iostream>

using namespace std;

namespace util
{
	/** Manages all SQLite operations. */
	template< typename Point >
	class SQLiteManager
	{
	public:
		SQLiteManager();
		~SQLiteManager();
		
		void insertPoint( const Point& point );
		Point getPoint( const sqlite3_uint64& index );
		
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
	};
	
	template< typename Point >
	SQLiteManager< Point >::SQLiteManager()
	: m_db( nullptr ),
	m_pointInsertionStmt( nullptr ),
	m_pointQuery( nullptr ),
	m_nodeInsertion( nullptr ),
	m_nodeQuery( nullptr ),
	m_nodeIntervalQuery( nullptr )
	{
		checkReturnCode(
			sqlite3_open_v2( "Octree.db", &m_db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX, NULL ),
			SQLITE_OK
		);
		
		createTables();
		createStmts();
	}
	
	template< typename Point >
	SQLiteManager< Point >::~SQLiteManager()
	{
		release();
	}
	
	template< typename Point >
	void SQLiteManager< Point >::insertPoint( const Point& point )
	{
		const void* blob = &point;
		checkReturnCode( sqlite3_bind_blob( m_pointInsertionStmt, 1, blob, sizeof( Point ), SQLITE_STATIC ), SQLITE_OK );
		safeStep( m_pointInsertionStmt );
		safeReset( m_pointInsertionStmt );
	}
	
	template< typename Point >
	Point SQLiteManager< Point >::getPoint( const sqlite3_uint64& index )
	{
		cout << "Reading point from db." << endl;
		checkReturnCode( sqlite3_bind_int64( m_pointQuery, 1, index ), SQLITE_OK );
		cout << "Found it? " << safeStep( m_pointQuery ) << endl;
		const void* blob = sqlite3_column_blob( m_pointQuery, 0 );
		Point* point = ( Point* ) blob;
		cout << *point << endl;
		safeReset( m_pointQuery );
		cout << "Casting blob to Point." << endl;
		return *point;
	}
	
	template< typename Point >
	void SQLiteManager< Point >::createTables()
	{
		sqlite3_stmt* creationStmt;
		
		safePrepare(
			"CREATE TABLE IF NOT EXISTS Points ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT,"
				"Point BLOB"
			");"
			"CREATE TABLE IF NOT EXISTS Nodes("
				"Morton INTEGER PRIMARY KEY,"
				"Node BLOB"
			");",
			&creationStmt
		);
		safeStep( creationStmt );
		safeFinalize( creationStmt );
	}
	
	template< typename Point >
	void SQLiteManager< Point >::dropTables()
	{
		sqlite3_stmt* dropStmt;
		
		unsafePrepare( "DROP TABLE IF EXISTS Points; DROP TABLE IF EXISTS Nodes;", &dropStmt );
		unsafeStep( dropStmt );
		unsafeFinalize( dropStmt );
	}
	
	template< typename Point >
	void SQLiteManager< Point >::createStmts()
	{
		safePrepare( "INSERT INTO Points( Point ) VALUES ( ? );", &m_pointInsertionStmt);
		safePrepare( "INSERT INTO Nodes VALUES ( ?, ? );", &m_nodeInsertion );
		safePrepare( "SELECT Point FROM Points WHERE Id = ?;", &m_pointQuery );
		safePrepare( "SELECT Node FROM Nodes WHERE Morton = ?;", &m_nodeQuery );
		safePrepare( "SELECT Node FROM Nodes WHERE Morton BETWEEN ? AND ?;", &m_nodeIntervalQuery );
	}
	
	template< typename Point >
	void SQLiteManager< Point >::release()
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
	
	template< typename Point >
	inline void SQLiteManager< Point >::checkReturnCode( const int& returnCode, const int& expectedCode )
	{
		if( returnCode != expectedCode )
		{
			throw runtime_error( sqlite3_errstr( returnCode ) );
		}
	}
	
	template< typename Point >
	inline void SQLiteManager< Point >::safePrepare( const char* stringStmt, sqlite3_stmt** stmt )
	{
		checkReturnCode( sqlite3_prepare_v2( m_db, stringStmt, -1, stmt, NULL ), SQLITE_OK );
	}
	
	template< typename Point >
	inline void SQLiteManager< Point >::unsafePrepare( const char* stringStmt, sqlite3_stmt** stmt )
	{
		sqlite3_prepare_v2( m_db, stringStmt, -1, stmt, NULL );
	}
	
	template< typename Point >
	inline bool SQLiteManager< Point >::safeStep( sqlite3_stmt* statement )
	{
		int returnCode = sqlite3_step( statement );
		switch( returnCode )
		{
			case SQLITE_ROW: return true;
			case SQLITE_DONE: return false;
			default: throw runtime_error( sqlite3_errstr( returnCode ) );
		}
	}
	
	template< typename Point >
	inline void SQLiteManager< Point >::unsafeStep( sqlite3_stmt* statement )
	{
		sqlite3_step( statement );
	}
	
	template< typename Point >
	inline void SQLiteManager< Point >::safeFinalize( sqlite3_stmt* statement )
	{
		if( statement )
		{
			checkReturnCode( sqlite3_finalize( statement ), SQLITE_OK );
		}
	}
	
	template< typename Point >
	inline void SQLiteManager< Point >::unsafeFinalize( sqlite3_stmt* statement )
	{
		if( statement )
		{
			sqlite3_finalize( statement );
		}
	}
	
	template< typename Point >
	inline void SQLiteManager< Point >::safeReset( sqlite3_stmt* statement )
	{
		checkReturnCode( sqlite3_reset( statement ), SQLITE_OK );
	}
}

#endif