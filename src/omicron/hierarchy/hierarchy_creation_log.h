#ifndef HIERARCHY_CREATOR_LOG_H
#define HIERARCHY_CREATOR_LOG_H

#include <fstream>
#include <mutex>

using namespace std;

namespace omicron::hierarchy
{
	class HierarchyCreationLog
	{
	public:
		/** Logs a debug message. */
		static void logDebugMsg( const string& msg )
		{
			lock_guard< recursive_mutex > lock( m_logMutex );
			m_log << msg;
			m_log.flush();
		}
	
		/** Logs a debug message and fail by throwing an exception. */
		static void logAndFail( const string& msg )
		{
			{
				lock_guard< recursive_mutex > lock( m_logMutex );
				logDebugMsg( msg );
				flush();
			}
			throw logic_error( msg );
		}
	
		static void flush()
		{
			m_log.flush();
		}
	
	private:
		static recursive_mutex m_logMutex;
		static ofstream m_log;
	};
}

#endif
