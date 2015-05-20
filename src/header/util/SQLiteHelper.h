#ifndef SQLITE_HELPER_H
#define SQLITE_HELPER_H

namespace util
{
	/** Class with methods to ease SQLite usage. */
	struct SQLiteHelper
	{
		/** Calls a SQLite function, checking the return code and throwing an runtime_error whenever appropriate. */
		static void safeCall( function< int() > sqliteFunc )
		{
			int returnCode = sqliteFunc();
			if( returnCode )
			{
				throw runtime_error( sqlite3_errstr( returnCode ) );
			}
		}
		
		/** Calls SQLite step function safely. */
		static void safeStep( function< int() > sqLiteStepFunc )
		{
			int returnCode = sqLiteStepFunc();
			if( returnCode != SQLITE_DONE )
			{
				throw runtime_error( sqlite3_errstr( returnCode ) );
			}
		}
	};
}

#endif