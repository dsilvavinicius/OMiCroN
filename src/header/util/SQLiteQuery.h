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
		 * It should return true if the step results in a row or false if the query is done.
		 * @param parser is the function that will parse the queried data into Queried type pointer. Also allocates the
		 * queried data.
		 * @disposer is the function that will clean-up resources associated with the query. */
		SQLiteQuery( const function< bool ( Queried** ) >& parser, const function< void () >& disposer );
		
		~SQLiteQuery();
		
		/** Steps into results.
		 * @param queried is the data acquired in the step.
		 * @returns true if the step returns data or false if the query is done. */
		bool step( Queried** queried );
		
		/** Disposes query resources. */
		void dispose();
		
	private:
		function< bool ( Queried** ) > m_parser;
		function< void () > m_disposer;
	};
	
	template< typename Queried >
	SQLiteQuery< Queried >::SQLiteQuery( const function< bool ( Queried** ) >& parser,
										 const function< void () >& disposer )
	: m_parser( parser ),
	m_disposer( disposer )
	{}
	
	template< typename Queried >
	SQLiteQuery< Queried >::~SQLiteQuery()
	{
		dispose();
	}
	
	template< typename Queried >
	inline bool SQLiteQuery< Queried >::step( Queried** queried )
	{
		return m_parser( queried );
	}
	
	template< typename Queried >
	inline void SQLiteQuery< Queried >::dispose()
	{
		m_disposer();
	}
	
	// ====================== Type Sugar ================================ /
	template< typename MortonCode >
	using IdNode = pair< MortonCode*, OctreeNode< MortonCode >* >;
	
	template< typename MortonCode >
	using IdNodeVector = vector< IdNode< MortonCode > >;
	
	using ShallowIdNodeSQLQuery = SQLiteQuery< IdNode< ShallowMortonCode > >;
}

#endif