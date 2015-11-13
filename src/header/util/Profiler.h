#ifndef PROFILING_H
#define PROFILING_H

#include <chrono>

using namespace std;

namespace util
{
	// Profiling methods.
	class Profiler
	{
	public:
		static chrono::system_clock::time_point now()
		{
			return std::chrono::high_resolution_clock::now();
		}
		
		static int elapsedTime( const chrono::system_clock::time_point& reference )
		{
			auto currentTime = now();
			return chrono::duration_cast< std::chrono::milliseconds >( currentTime - reference ).count();
		}
	};
}

#endif