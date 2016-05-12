#ifndef STACK_TRACE_H
#define STACK_TRACE_H

#include <iostream>
#include <mutex>
#include <execinfo.h>
#include "HierarchyCreationLog.h"

using namespace std;
using namespace model;

namespace util
{
	class StackTrace
	{
	public:
		static void print( ostream& out, recursive_mutex& mutex )
		{
			lock_guard< recursive_mutex > lock( mutex );
			
			out << toString();
		}
		
		static void log()
		{
			stringstream ss;
			ss << toString();
			HierarchyCreationLog::logDebugMsg( ss.str() );
			HierarchyCreationLog::flush();
		}
		
		/** Handler for system signals. */
		static void handler( int )
		{
			log();
			exit( 1 );
		}
		
		static string toString()
		{
			constexpr const int STACK_SIZE = 100;
			void* buffer[ STACK_SIZE ];
			int entries = backtrace( buffer, STACK_SIZE );
			char** strings = backtrace_symbols( buffer, entries );
			
			stringstream ss;
			for( int i = 0; i < entries; ++i )
			{
				ss << strings[ i ] << endl;
			}
			ss << endl;
			
			free( strings );
			return ss.str();
		}
	};
}

#endif