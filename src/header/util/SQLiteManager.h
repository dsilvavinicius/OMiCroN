#ifndef SQLITE_MANAGER_H
#define SQLITE_MANAGER_H

#include <stdexcept>
#include <functional>
#include <sqlite3.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <unordered_set>
#include <queue>
#include "SQLiteQuery.h"
#include "MortonInterval.h"

using namespace std;

namespace util
{
	/** Manages all SQLite operations. Provides a sync and async API. The async API can be used to make node requests
	 * and to acquire them later on. */
	template< typename Point, typename MortonCode, typename OctreeNode >
	class SQLiteManager
	{
	public:
		using MortonCodePtr = shared_ptr< MortonCode >;
		using MortonInterval = model::MortonInterval< MortonCode >;
		
		using PointPtr = shared_ptr< Point >;
		using PointVector = vector< PointPtr, ManagedAllocator< PointPtr > >;
		
		using OctreeNodePtr = shared_ptr< OctreeNode >;
		
		using IdNode = model::IdNode< MortonCode, OctreeNode >;
		using IdNodeVector = vector< IdNode >;
		using ManagedIdNode = model::ManagedIdNode< MortonCode, OctreeNode >;
		using ManagedIdNodeVector = vector< ManagedIdNode >;
		using SQLiteQuery = util::SQLiteQuery< ManagedIdNode >;
		using unordered_set = std::unordered_set< MortonInterval, hash< MortonInterval >,
												  MortonIntervalComparator< MortonInterval > >;
		
		SQLiteManager( const string& dbFileName );
		
		~SQLiteManager();
		
		/** Indicates that the next operations will be part of a transaction until endTransaction() is called.
		 * Transaction should be used to improve performance. */
		void beginTransaction();
		
		/** Finishes the current transaction. */
		void endTransaction();
		
		/** Inserts point into database.
		 *	@returns the index of the pointer. The first index is 0 and each next point increments it by 1. */
		sqlite_int64 insertPoint( const Point& point );
		
		/** Searches the point in the database, given its index.
		 *	@return the acquired node or nullptr if no point is found. */
		PointPtr getPoint( const sqlite3_uint64& index );
		
		/** Inserts the node into database, using the given morton code as identifier. */
		void insertNode( const MortonCode& morton, const OctreeNode& node );
		
		/** Searches the node in the database, given its morton code.
		 *	@param morton is the node morton code id.
		 *	@returns the acquired node or nullptr if no node is found. */
		OctreeNodePtr getManagedNode( const MortonCode& morton );
		
		/** Searches the node in the database, given its morton code.
		 *	@param morton is the node morton code id.
		 *	@returns a pair in which the first element indicates if the node was found and the second is the acquired
		 * node or a default constructed node if not found. */
		pair< bool, OctreeNode > getNode( const MortonCode& morton );
		
		/** Searches for a range of nodes in the database, given the morton code interval [a, b].
		 *	@param a is the minor boundary of the morton code closed interval.
		 *	@param b is the major boundary of the morton closed interval.
		 *	@returns the acquired nodes. */
		vector< OctreeNodePtr > getManagedNodes( const MortonCode& a, const MortonCode& b );
		
		/** Searches for a range of nodes in the database, given the morton code interval [a, b].
		 *	@param a is the minor boundary of the morton code closed interval.
		 *	@param b is the major boundary of the morton closed interval.
		 *	@returns the acquired nodes. */
		vector< OctreeNode > getNodes( const MortonCode& a, const MortonCode& b );
		
		/** Returns all nodes in database. */
		ManagedIdNodeVector getManagedIdNodes();
		
		/** Searches for a range of nodes in the database, given the morton code interval [a, b].
		 *	@param a is the minor boundary of the morton code closed interval.
		 *	@param b is the major boundary of the morton closed interval.
		 *	@returns pairs of acquired nodes and morton id. The ownership of the node pointers is caller's. */
		ManagedIdNodeVector getManagedIdNodes( const MortonCode& a, const MortonCode& b );
		
		/** Returns a query for all nodes in database. Appropriate for big queries. */
		SQLiteQuery getManagedIdNodesQuery();
		
		/** Searches for a range of nodes in the database, given the morton code interval [a, b]. This version returns
		 * a query for stepping and should be used for probable big queries or to avoid the performance penalty of
		 * creating a result vector.
		 *	@param a is the minor boundary of the morton code closed interval.
		 *	@param b is the major boundary of the morton closed interval.
		 *	@returns a query that can be iterated for IdNodeAcquisition. */
		SQLiteQuery getManagedIdNodesQuery( const MortonCode& a, const MortonCode& b );
		
		/** Deletes a range of nodes in the database, given the morton code interval [ a, b ]
		 * @param a is the minor boundary of the morton code closed interval.
		 * @param b is the major boundary of the morton closed interval.*/
		void deleteNodes( const MortonCode& a, const MortonCode& b );
		
		/** Async request the closed interval of nodes. They will be available later on and can be acessed using
		 * getRequestResults(). */
		void requestNodesAsync( const MortonInterval& interval );
		
		/** Get results available from earlier async node requests.
		 * @param maxResults maximum number of results to be returned.
		 * @returns a vector which each entry represents the results of one async request. */
		vector< ManagedIdNodeVector > getRequestResults( const unsigned int& maxResults );
		
		string output();
		
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
		
		/** Method used to step IdNode queries. */
		bool stepIdNodeQuery( ManagedIdNode* queried, sqlite3_stmt* stmt );
		
		sqlite3* m_db;
		
		sqlite3_stmt* m_pointInsertion;
		sqlite3_stmt* m_pointQuery;
		
		sqlite3_stmt* m_nodeInsertion;
		sqlite3_stmt* m_nodeQuery;
		sqlite3_stmt* m_nodeIntervalQuery;
		sqlite3_stmt* m_allIdNodesQuery;
		sqlite3_stmt* m_nodeIntervalIdQuery;
		sqlite3_stmt* m_nodeIntervalDeletion;
		sqlite3_stmt* m_beginTransaction;
		sqlite3_stmt* m_endTransaction;
		
		/** Current number of inserted points. */
		sqlite_int64 m_nPoints;
		
		// Async request manager associated data:
		
		// The manager itself.
		thread m_requestManager;
		
		// Lock to the request container.
		mutex m_requestLock;
		
		// Request container.
		unordered_set m_requests;
		
		// Lock to the result container.
		mutex m_resultLock;
		
		// Requests result container.
		queue< ManagedIdNodeVector > m_requestsResults;
		
		// Indicates that request were generated for requestManager.
		condition_variable m_requestFlag;
		
		// Indicates that all async requests are sent to requestManager and no more will be generated. requestManager
		// should then finish processing all request and join with main thread.
		bool m_requestsDone;
	};
	
	template< typename Point, typename MortonCode, typename OctreeNode >
	SQLiteManager< Point, MortonCode, OctreeNode >::SQLiteManager( const string& dbFileName )
	: m_db( nullptr ),
	m_pointInsertion( nullptr ),
	m_pointQuery( nullptr ),
	m_nodeInsertion( nullptr ),
	m_nodeQuery( nullptr ),
	m_nodeIntervalQuery( nullptr ),
	m_allIdNodesQuery( nullptr ),
	m_nodeIntervalIdQuery( nullptr ),
	m_nodeIntervalDeletion( nullptr ),
	m_beginTransaction( nullptr ),
	m_endTransaction( nullptr ),
	m_nPoints( 0 ),
	m_requestsDone( false )
	{
		checkReturnCode(
			sqlite3_open_v2( dbFileName.c_str(), &m_db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX,
							 NULL ),
			SQLITE_OK
		);
		sqlite3_exec( m_db, "PRAGMA synchronous = OFF", NULL, NULL, NULL );
		sqlite3_exec( m_db, "PRAGMA journal_mode = MEMORY", NULL, NULL, NULL );
		
		dropTables();
		createTables();
		createStmts();
		
		m_requestManager = thread(
			[ & ]()
			{
				while( !m_requestsDone )
				{
					unique_lock< mutex > requestLock( m_requestLock );
					m_requestFlag.wait( requestLock, [ & ](){ return !m_requests.empty() || m_requestsDone; } );
					
					for( auto it = m_requests.begin(); it != m_requests.end(); )
					{
						MortonInterval interval = *it;
						
						ManagedIdNodeVector idNodes = getManagedIdNodes( *interval.first, *interval.second );
						{
							lock_guard< mutex > resultLock( m_resultLock );
							m_requestsResults.push( idNodes );
						}
						m_requests.erase(it++);
					}
				}
			}
		);
	}
	
	template< typename Point, typename MortonCode, typename OctreeNode >
	SQLiteManager< Point, MortonCode, OctreeNode >::~SQLiteManager()
	{
		release();
	}
	
	template< typename Point, typename MortonCode, typename OctreeNode >
	void SQLiteManager< Point, MortonCode, OctreeNode >::beginTransaction()
	{
		safeStep( m_beginTransaction );
		safeReset( m_beginTransaction );
	}
	
	template< typename Point, typename MortonCode, typename OctreeNode >
	void SQLiteManager< Point, MortonCode, OctreeNode >::endTransaction()
	{
		safeStep( m_endTransaction );
		safeReset( m_endTransaction );
	}
	
	template< typename Point, typename MortonCode, typename OctreeNode >
	sqlite_int64 SQLiteManager< Point, MortonCode, OctreeNode >::insertPoint( const Point& point )
	{
		byte* serialization;
		size_t blobSize = point.serialize( &serialization );
		
		checkReturnCode( sqlite3_bind_int64( m_pointInsertion, 1, m_nPoints ), SQLITE_OK );
		checkReturnCode( sqlite3_bind_blob( m_pointInsertion, 2, serialization, blobSize, SQLITE_STATIC ), SQLITE_OK );
		safeStep( m_pointInsertion );
		safeReset( m_pointInsertion );
		
		delete[] serialization;
		
		return m_nPoints++;
	}
	
	template< typename Point, typename MortonCode, typename OctreeNode >
	shared_ptr< Point > SQLiteManager< Point, MortonCode, OctreeNode >::getPoint( const sqlite3_uint64& index )
	{
		checkReturnCode( sqlite3_bind_int64( m_pointQuery, 1, index ), SQLITE_OK );
		bool isPointFound = safeStep( m_pointQuery );
		
		PointPtr point = nullptr;
		
		if( isPointFound )
		{
			byte* blob = ( byte* ) sqlite3_column_blob( m_pointQuery, 0 );
			point = makeManaged< Point >( blob );
		}
		else
		{
			cout << "Cannot find point with index " << index << endl << endl;
		}
		
		safeReset( m_pointQuery );
		return point;
	}
	
	template< typename Point, typename MortonCode, typename OctreeNode >
	void SQLiteManager< Point, MortonCode, OctreeNode >::insertNode( const MortonCode& morton, const OctreeNode& node )
	{
		byte* serialization;
		size_t blobSize = node.serialize( &serialization );
		
		checkReturnCode( sqlite3_bind_int64( m_nodeInsertion, 1, morton.getBits() ), SQLITE_OK );
		checkReturnCode( sqlite3_bind_blob( m_nodeInsertion, 2, serialization, blobSize, SQLITE_STATIC ), SQLITE_OK );
		
		safeStep( m_nodeInsertion );
		safeReset( m_nodeInsertion );
		
		delete[] serialization;
	}
	
	template< typename Point, typename MortonCode, typename OctreeNode >
	pair< bool, OctreeNode > SQLiteManager< Point, MortonCode, OctreeNode >::getNode( const MortonCode& morton )
	{
		checkReturnCode( sqlite3_bind_int64( m_nodeQuery, 1, morton.getBits() ), SQLITE_OK );
		
		if( safeStep( m_nodeQuery ) )
		{
			byte* blob = ( byte* ) sqlite3_column_blob( m_nodeQuery, 0 );
			auto query = pair< bool, OctreeNode >( true, OctreeNode::deserialize( blob ) );
			safeReset( m_nodeQuery );
			return std::move( query );
		}
		
		safeReset( m_nodeQuery );
		return pair< bool, OctreeNode >( false, OctreeNode() );
	}
	
	template< typename Point, typename MortonCode, typename OctreeNode >
	shared_ptr< OctreeNode > SQLiteManager< Point, MortonCode, OctreeNode >::getManagedNode( const MortonCode& morton )
	{
		OctreeNodePtr node = nullptr;
		
		pair< bool, OctreeNode > query = getNode( morton );
		
		if( query.first )
		{
			return makeManaged< OctreeNode >( std::move( query.second ) );
		}
		else
		{
			return nullptr;
		}
	}
	
	template< typename Point, typename MortonCode, typename OctreeNode >
	vector< shared_ptr< OctreeNode > > SQLiteManager< Point, MortonCode, OctreeNode >
	::getManagedNodes( const MortonCode& a, const MortonCode& b )
	{
		checkReturnCode( sqlite3_bind_int64( m_nodeIntervalQuery, 1, a.getBits() ), SQLITE_OK );
		checkReturnCode( sqlite3_bind_int64( m_nodeIntervalQuery, 2, b.getBits() ), SQLITE_OK );
		
		vector< OctreeNodePtr > nodes;
		
		while( safeStep( m_nodeIntervalQuery ) )
		{
			byte* blob = ( byte* ) sqlite3_column_blob( m_nodeIntervalQuery, 0 );
			OctreeNodePtr node = makeManaged< OctreeNode >( OctreeNode::deserialize( blob ) );
			nodes.push_back( node );
		}
		
		safeReset( m_nodeIntervalQuery );
		return nodes;
	}
	
	template< typename Point, typename MortonCode, typename OctreeNode >
	ManagedIdNodeVector< MortonCode, OctreeNode > SQLiteManager< Point, MortonCode, OctreeNode >
	::getManagedIdNodes()
	{
		SQLiteQuery query = getManagedIdNodesQuery();
		ManagedIdNodeVector queried;
		
		ManagedIdNode idNode;
		while( query.step( &idNode ) )
		{
			queried.push_back( idNode );
		}
		
		return queried;
	}
	
	template< typename Point, typename MortonCode, typename OctreeNode >
	ManagedIdNodeVector< MortonCode, OctreeNode > SQLiteManager< Point, MortonCode, OctreeNode >
	::getManagedIdNodes( const MortonCode& a, const MortonCode& b )
	{
		SQLiteQuery query = getManagedIdNodesQuery( a, b );
		ManagedIdNodeVector queried;
		
		ManagedIdNode idNode;
		while( query.step( &idNode ) )
		{
			queried.push_back( idNode );
		}
		
		return queried;
	}
	
	template< typename Point, typename MortonCode, typename OctreeNode >
	SQLiteQuery< ManagedIdNode< MortonCode, OctreeNode > > SQLiteManager< Point, MortonCode, OctreeNode >
	::getManagedIdNodesQuery()
	{
		SQLiteQuery query(
			[ & ] ( ManagedIdNode* queried )
			{
				return stepIdNodeQuery( queried, m_allIdNodesQuery );
			},
			[ & ] () { safeReset( m_allIdNodesQuery ); }
		);
		
		return query;
	}
	
	template< typename Point, typename MortonCode, typename OctreeNode >
	SQLiteQuery< ManagedIdNode< MortonCode, OctreeNode > > SQLiteManager< Point, MortonCode, OctreeNode >
	::getManagedIdNodesQuery( const MortonCode& a, const MortonCode& b )
	{
		assert( a <= b && "a <= b should holds in order to define an interval." );
		
		checkReturnCode( sqlite3_bind_int64( m_nodeIntervalIdQuery, 1, a.getBits() ), SQLITE_OK );
		checkReturnCode( sqlite3_bind_int64( m_nodeIntervalIdQuery, 2, b.getBits() ), SQLITE_OK );
		
		SQLiteQuery query(
			[ & ] ( ManagedIdNode* queried )
			{
				return stepIdNodeQuery( queried, m_nodeIntervalIdQuery );
			},
			[ & ] () { safeReset( m_nodeIntervalIdQuery ); }
		);
		
		return query;
	}
	
	template< typename Point, typename MortonCode, typename OctreeNode >
	void SQLiteManager< Point, MortonCode, OctreeNode >::deleteNodes( const MortonCode& a, const MortonCode& b )
	{
		checkReturnCode( sqlite3_bind_int64( m_nodeIntervalDeletion, 1, a.getBits() ), SQLITE_OK );
		checkReturnCode( sqlite3_bind_int64( m_nodeIntervalDeletion, 2, b.getBits() ), SQLITE_OK );
		
		safeStep( m_nodeIntervalDeletion );
		safeReset( m_nodeIntervalDeletion );
	}
	
	template< typename Point, typename MortonCode, typename OctreeNode >
	void SQLiteManager< Point, MortonCode, OctreeNode >::requestNodesAsync( const MortonInterval& interval )
	{
		unique_lock< mutex > locker( m_requestLock );
		m_requests.insert( interval );
		locker.unlock();
		m_requestFlag.notify_one();
	}
	
	template< typename Point, typename MortonCode, typename OctreeNode >
	vector< ManagedIdNodeVector< MortonCode, OctreeNode > > SQLiteManager< Point, MortonCode, OctreeNode >
	::getRequestResults( const unsigned int& maxResults )
	{
		lock_guard< mutex > locker( m_resultLock );
		vector< ManagedIdNodeVector > results;
		
		for( unsigned int i = 0; i < maxResults && !m_requestsResults.empty(); ++i )
		{
			results.push_back( m_requestsResults.front() );
			m_requestsResults.pop();
		}
		
		return results;
	}
	
	template< typename Point, typename MortonCode, typename OctreeNode >
	string SQLiteManager< Point, MortonCode, OctreeNode >::output()
	{
		ManagedIdNodeVector nodes = getManagedIdNodes();
		stringstream ss;
		ss  << "=== SQLite ===" << endl << endl
			<< "Size: " << nodes.size() << endl << endl;
		for( ManagedIdNode node : nodes )
		{
			ss << node.first->getPathToRoot( true ) << endl;
		}
		ss << "=== End of SQLite ===" << endl;
		return ss.str();
	}
	
	template< typename Point, typename MortonCode, typename OctreeNode >
	void SQLiteManager< Point, MortonCode, OctreeNode >::createTables()
	{
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
	}
	
	template< typename Point, typename MortonCode, typename OctreeNode >
	void SQLiteManager< Point, MortonCode, OctreeNode >::dropTables()
	{
		sqlite3_stmt* dropStmt;
		
		unsafePrepare( "DROP TABLE IF EXISTS Points;", &dropStmt );
		unsafeStep( dropStmt );
		unsafeFinalize( dropStmt );
		
		unsafePrepare( "DROP TABLE IF EXISTS Nodes;", &dropStmt );
		unsafeStep( dropStmt );
		unsafeFinalize( dropStmt );
	}
	
	template< typename Point, typename MortonCode, typename OctreeNode >
	void SQLiteManager< Point, MortonCode, OctreeNode >::createStmts()
	{
		safePrepare( "INSERT OR REPLACE INTO Points VALUES ( ?, ? );", &m_pointInsertion );
		safePrepare( "INSERT OR REPLACE INTO Nodes VALUES ( ?, ? );", &m_nodeInsertion );
		safePrepare( "SELECT Point FROM Points WHERE Id = ?;", &m_pointQuery );
		safePrepare( "SELECT Node FROM Nodes WHERE Morton = ?;", &m_nodeQuery );
		safePrepare( "SELECT Node FROM Nodes WHERE Morton BETWEEN ? AND ?;", &m_nodeIntervalQuery );
		safePrepare( "SELECT * FROM Nodes;", &m_allIdNodesQuery );
		safePrepare( "SELECT Morton, Node FROM Nodes WHERE Morton BETWEEN ? AND ?;", &m_nodeIntervalIdQuery );
		safePrepare( "DELETE FROM Nodes WHERE Morton BETWEEN ? AND ?;", &m_nodeIntervalDeletion );
		safePrepare( "BEGIN TRANSACTION", &m_beginTransaction );
		safePrepare( "END TRANSACTION", &m_endTransaction );
	}
	
	template< typename Point, typename MortonCode, typename OctreeNode >
	void SQLiteManager< Point, MortonCode, OctreeNode >::release()
	{
		m_requestsDone = true;
		m_requestFlag.notify_one();
		m_requestManager.join();
		
		dropTables();
		
		unsafeFinalize( m_pointInsertion );
		unsafeFinalize( m_pointQuery );
		unsafeFinalize( m_nodeInsertion );
		unsafeFinalize( m_nodeQuery );
		unsafeFinalize( m_nodeIntervalQuery );
		unsafeFinalize( m_allIdNodesQuery );
		unsafeFinalize( m_nodeIntervalIdQuery );
		unsafeFinalize( m_nodeIntervalDeletion );
		unsafeFinalize( m_beginTransaction );
		unsafeFinalize( m_endTransaction );
		
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
	
	template< typename Point, typename MortonCode, typename OctreeNode >
	bool SQLiteManager< Point, MortonCode, OctreeNode >::stepIdNodeQuery( ManagedIdNode* queried, sqlite3_stmt* stmt )
	{
		bool rowIsFound = safeStep( stmt );
		if( rowIsFound )
		{
			sqlite3_int64 mortonBits = sqlite3_column_int64( stmt, 0 );
			MortonCodePtr code = makeManaged< MortonCode >();
			code->build( mortonBits );
			
			byte* blob = ( byte* ) sqlite3_column_blob( stmt, 1 );
			OctreeNodePtr node = makeManaged< OctreeNode >( OctreeNode::deserialize( blob ) );
			
			*queried = ManagedIdNode( code, node );
		}
		else
		{
			*queried = ManagedIdNode( nullptr, nullptr );
		}
		
		return rowIsFound;
	}
}

#endif