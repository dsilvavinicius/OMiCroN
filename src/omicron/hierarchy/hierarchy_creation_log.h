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
		static void logDebugMsg( const string& msg, ostream& out = m_log )
		{
			lock_guard< recursive_mutex > lock( m_logMutex );
			out << msg;
		}
	
		/** Logs a debug message and fail by throwing an exception. */
		static void logAndFail( const string& msg, ostream& out = m_log )
		{
			{
				lock_guard< recursive_mutex > lock( m_logMutex );
				logDebugMsg( msg, out );
				flush();
			}
			throw logic_error( msg );
		}
	
		static void flush(ostream& out = m_log)
		{
			out.flush();
		}
	
	private:
		static recursive_mutex m_logMutex;
		static ofstream m_log;
	};
}

#endif
