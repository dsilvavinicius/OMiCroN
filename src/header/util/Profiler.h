#ifndef PROFILING_H
#define PROFILING_H

#include <chrono>
#include <iostream>

using namespace std;

namespace util
{
	// Profiling methods.
	class Profiler
	{
	public:
		/** @returns the clock time now. */
		static chrono::system_clock::time_point now()
		{
			return chrono::high_resolution_clock::now();
		}
		
		/** Logs the task starting time.
		 * @returns the clock time now. */
		static chrono::system_clock::time_point now( const string& taskName, ostream& log = cout )
		{
			auto timestamp = now();
			
			std::time_t now = chrono::high_resolution_clock::to_time_t( timestamp );
			log << taskName << " started at: " << ctime( &now ) << endl;
			
			return timestamp;
		}
		
		/** @returns the elapsed time in milliseconds from a reference. */
		static int elapsedTime( const chrono::system_clock::time_point& reference )
		{
			auto timestamp = now();
			return chrono::duration_cast< std::chrono::milliseconds >( timestamp - reference ).count();
		}
		
		/** Logs the task finish time and elapsed time from a given reference. 
		 * @returns the elapsed time in milliseconds from a reference. */
		static int elapsedTime( const chrono::system_clock::time_point& reference, const string& taskName,
								ostream& log = cout  )
		{
			auto timestamp = now();
			int duration = chrono::duration_cast< std::chrono::milliseconds >( timestamp - reference ).count();
			
			std::time_t now = chrono::high_resolution_clock::to_time_t( timestamp );
			log << taskName << " finished at: " << ctime( &now ) << endl << "Duration: " << duration << " ms." << endl
				<< endl;
			
			return duration;
		}
		
		/** @returns the time difference in milliseconds of the two references. */
		static int elapsedTime( const chrono::system_clock::time_point& earlier,
								const chrono::system_clock::time_point& after )
		{
			return chrono::duration_cast< std::chrono::milliseconds >( after - earlier ).count();
		}
		
		/** Logs the duration of a task.
		 * @returns the time difference in milliseconds of the two references. */
		static int elapsedTime( const chrono::system_clock::time_point& earlier,
								const chrono::system_clock::time_point& after, const string& taskName,
								ostream& log = cout  )
		{
			int duration = chrono::duration_cast< std::chrono::milliseconds >( after - earlier ).count();
			log << taskName << " spams: " << duration << " ms." << endl << endl;
			
			return duration;
		}
	};
}

#endif