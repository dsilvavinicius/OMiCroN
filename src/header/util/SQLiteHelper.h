#ifndef SQLITE_HELPER_H
#define SQLITE_HELPER_H

namespace util
{
	/** Class with methods to ease SQLite usage. */
	class SQLiteHelper
	{
		/** Calls a SQLite function, checking the return code and throwing an runtime_error whenever appropriate. */
		static void safeCall( function< int()  > sqliteFunc );
	};
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point  >
	inline void OutOfCoreOctree< MortonPrecision, Float, Vec3, Point >::safeCall( function< int()  > sqliteFunc )
	{
		int returnCode = sqliteFunc();
		if( returnCode )
		{
			throw runtime_error( sqlite3_errmsg( m_db ) );
		}
	}
}

#endif