#ifndef SQLITE_QUERY_H
#define SQLITE_QUERY_H

#include <sqlite3.h>
#include <functional>
#include "MortonCode.h"
#include "OctreeNode.h"

using namespace std;
using namespace model;

namespace util
{
	/** Probable big query class. Instead of returning all results in a vector as the methods in SQLiteManager, provides
	 * API for stepping the results. */
	template< typename Queried >
	class SQLiteQuery
	{
	public:
		/** The parser function should be aware of the queried final data type and must parse the result to it.
		 * It should return true if the step results in a row or false if the query is done. */
		SQLiteQuery( const function< bool ( Queried* ) >& parser );
		bool step( Queried* queried );
		
	private:
		function< bool ( Queried* ) > m_parser;
	};
	
	template< typename Queried >
	SQLiteQuery< Queried >::SQLiteQuery( const function< bool ( Queried* ) >& parser )
	: m_parser( parser )
	{}
	
	template< typename Queried >
	bool SQLiteQuery< Queried >::step( Queried* queried )
	{
		return m_parser( queried );
	}
	
	// ====================== Type Sugar ================================ /
	template< typename MortonCode >
	using IdNode = pair< MortonCode*, OctreeNode< MortonCode >* >;
	
	template< typename MortonCode >
	using IdNodeVector = vector< IdNode< MortonCode > >;
	
	using ShallowIdNodeSQLQuery = SQLiteQuery< IdNode< ShallowMortonCode > >;
}

#endif