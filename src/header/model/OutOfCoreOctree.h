#ifndef OUT_OF_CORE_OCTREE_H
#define OUT_OF_CORE_OCTREE_H

#include <sqlite3.h>
#include <stdexcept>
#include <functional>
#include "FrontOctree.h"
#include "OutOfCorePlyPointReader.h"
#include "SQLiteHelper.h"

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
		using ParentOctree = model::FrontOctree< MortonPrecision, Float, Vec3, Point, Front, FrontInsertionContainer >;
		
	public:
		OutOfCoreOctree( const int& maxPointsPerNode, const int& maxLevel );
		~OutOfCoreOctree();
		
		void buildFromFile( const string& plyFileName, const typename PlyPointReader::Precision& precision, const Attributes& attribs ) override;
		
	private:
		sqlite3* m_db;
		sqlite3_stmt* m_nodeInsertion;
		sqlite3_stmt* m_pointQuery;
		sqlite3_stmt* m_nodeQuery;
		sqlite3_stmt* m_nodeIntervalQuery;
	};
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point, typename Front,
			  typename FrontInsertionContainer >
	OutOfCoreOctree< MortonPrecision, Float, Vec3, Point, Front, FrontInsertionContainer >::
	OutOfCoreOctree( const int& maxPointsPerNode, const int& maxLevel )
	: ParentOctree( maxPointsPerNode, maxLevel )
	{
		cout << "Before creating db." << endl;
		
		// Creating database, hierarchy and point tables.
		SQLiteHelper::safeCall(
			[ & ] ()
			{
				return sqlite3_open_v2( "Octree.db", &m_db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX,
										NULL );
				
			}
		);
		
		sqlite3_stmt* creationStmt;
		
		cout << "Before dropping point table" << endl;
		
		SQLiteHelper::safeCall(
			[ & ] () { return sqlite3_prepare_v2( m_db, "DROP TABLE IF EXISTS Points;", -1, &creationStmt, NULL ); }
		);
		SQLiteHelper::safeStep( [ & ] () { return sqlite3_step( creationStmt ); } );
		SQLiteHelper::safeCall( [ & ] () { return sqlite3_finalize( creationStmt ); } );
		
		cout << "Before creating point table" << endl;
		
		SQLiteHelper::safeCall(
			[ & ] ()
			{
			return sqlite3_prepare_v2( 	m_db,
										"CREATE TABLE IF NOT EXISTS Points ("
											"Id INTEGER PRIMARY KEY AUTOINCREMENT,"
											"Point BLOB"
										");",
										-1, &creationStmt, NULL
									);
			}
		);
		SQLiteHelper::safeStep( [ & ] () { return sqlite3_step( creationStmt ); } );
		SQLiteHelper::safeCall( [ & ] () { return sqlite3_finalize( creationStmt ); } );
		
		cout << "Before dropping node table" << endl;
		
		SQLiteHelper::safeCall(
			[ & ] ()
			{
				return sqlite3_prepare_v2( m_db, "DROP TABLE IF EXISTS Nodes;", -1, &creationStmt, NULL);
			}
		);
		SQLiteHelper::safeStep( [ & ] () { return sqlite3_step( creationStmt ); } );
		SQLiteHelper::safeCall( [ & ] () { return sqlite3_finalize( creationStmt ); } );
		
		cout << "Before creating node table." << endl;
		
		SQLiteHelper::safeCall(
			[ & ] ()
			{
				return sqlite3_prepare_v2( m_db,
										   "CREATE TABLE IF NOT EXISTS Nodes(Morton INTEGER PRIMARY KEY, Node BLOLB);", -1,
										   &creationStmt, NULL);
			}
		);
		SQLiteHelper::safeStep( [ & ] () { return sqlite3_step( creationStmt ); } );
		SQLiteHelper::safeCall( [ & ] () { return sqlite3_finalize( creationStmt ); } );
		
		cout << "Before creating insert stmt0." << endl;
		
		// Creating insertion statements.
		SQLiteHelper::safeCall(
			[ & ] ()
			{
				return sqlite3_prepare_v2( m_db, "INSERT INTO Nodes VALUES ( ?, ? );", -1, &m_nodeInsertion, NULL );
			}
		);
		
		cout << "Before creating insert stmt1." << endl;
		
		// Creating queries.
		SQLiteHelper::safeCall(
			[ & ] ()
			{
				return sqlite3_prepare_v2( m_db, "SELECT Point FROM Points WHERE Id = ?;", -1, &m_pointQuery, NULL );
			}
		);
		
		cout << "Before creating selection stmt0." << endl;
		
		SQLiteHelper::safeCall(
			[ & ] ()
			{
				return sqlite3_prepare_v2( m_db, "SELECT Node FROM Nodes WHERE Morton = ?;", -1, &m_nodeQuery, NULL );
			}
		);
		
		cout << "Before creating selection stmt1." << endl;
		
		SQLiteHelper::safeCall(
			[ & ] ()
			{
				return sqlite3_prepare_v2( m_db, "SELECT Node FROM Nodes WHERE Morton BETWEEN ? AND ?;", -1,
										   &m_nodeIntervalQuery, NULL );
			}
		);
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point, typename Front,
			  typename FrontInsertionContainer >
	OutOfCoreOctree< MortonPrecision, Float, Vec3, Point, Front, FrontInsertionContainer >::~OutOfCoreOctree()
	{
		SQLiteHelper::safeCall( [ & ] { return sqlite3_finalize( m_pointQuery ); } );
		SQLiteHelper::safeCall( [ & ] { return sqlite3_finalize( m_nodeIntervalQuery ); } );
		SQLiteHelper::safeCall( [ & ] { return sqlite3_close( m_db ); } );
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point, typename Front,
			  typename FrontInsertionContainer >
	void OutOfCoreOctree< MortonPrecision, Float, Vec3, Point, Front, FrontInsertionContainer >
	::buildFromFile( const string& plyFileName, const typename PlyPointReader::Precision& precision, const Attributes& attribs )
	{
		auto *reader = new PlyPointReader( m_db );
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