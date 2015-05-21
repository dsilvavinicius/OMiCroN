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
		
		/** Calls sqlite3_step function safely. */
		void safeStep( sqlite3_stmt* statement );
		
		/** Calls sqlite3_step function unsafely. */
		void unsafeStep( sqlite3_stmt* statement );
		
		/** Safe finalizes an sqlite3_stmt. */
		void safeFinalize( sqlite3_stmt* statement );
		
		/** Safe finalizes an sqlite3_stmt. */
		void unsafeFinalize( sqlite3_stmt* statement );
		
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
		cout << "Before creating db." << endl;
		
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
		cout << "Inserting " << point << endl;
			
		const void* blob = &point;
		checkReturnCode( sqlite3_bind_blob( m_pointInsertionStmt, 1, blob, sizeof( Point ), SQLITE_STATIC ),
							SQLITE_OK );
		safeStep( m_pointInsertionStmt );
	}
	
	template< typename Point >
	void SQLiteManager< Point >::createTables()
	{
		sqlite3_stmt* creationStmt;
		
		cout << "Before creating point table" << endl;
		
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
		cout << "Before dropping tables" << endl;
		
		sqlite3_stmt* dropStmt;
		
		unsafePrepare( "DROP TABLE IF EXISTS Points; DROP TABLE IF EXISTS Nodes;", &dropStmt );
		unsafeStep( dropStmt );
		unsafeFinalize( dropStmt );
	}
	
	template< typename Point >
	void SQLiteManager< Point >::createStmts()
	{
		cout << "Before creating point insertion stmt." << endl;
		
		safePrepare( "INSERT INTO Points( Point ) VALUES ( ? );", &m_pointInsertionStmt);
		
		cout << "Before creating node insertion stmt." << endl;
		
		safePrepare( "INSERT INTO Nodes VALUES ( ?, ? );", &m_nodeInsertion );
		
		cout << "Before creating point selection stmt." << endl;
		
		safePrepare( "SELECT Point FROM Points WHERE Id = ?;", &m_pointQuery );
		
		cout << "Before creating node selection stmt." << endl;
		
		safePrepare( "SELECT Node FROM Nodes WHERE Morton = ?;", &m_nodeQuery );
		
		cout << "Before creating node interval selection stmt." << endl;
		
		safePrepare( "SELECT Node FROM Nodes WHERE Morton BETWEEN ? AND ?;", &m_nodeIntervalQuery );
	}
	
	template< typename Point >
	void SQLiteManager< Point >::release()
	{
		cout << "Releasing SQLite resources." << endl;
		
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
	inline void SQLiteManager< Point >::safeStep( sqlite3_stmt* statement )
	{
		checkReturnCode( sqlite3_step( statement ), SQLITE_DONE );
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
}

#endif