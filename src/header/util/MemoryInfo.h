/*
 * Author:		David Robert Nadeau
 * Site:		http://NadeauSoftware.com/
 * License:		Creative Commons Attribution 3.0 Unported License
 *          	http://creativecommons.org/licenses/by/3.0/deed.en_US
 *
 * Port for C++ and getAvailableMemorySize() by Vinícius da Silva.
 * http://www.lcg.ufrj.br/Members/viniciusdasilva
 */

#ifndef MEMORY_INFO_H
#define MEMORY_INFO_H

#include <iostream>
#include <fstream>
#include <stdexcept>

#if defined(_WIN32)
#include <Windows.h>

#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
#include <unistd.h>
#include <sys/types.h>
#include <sys/param.h>
#include <string.h>
#include <sys/sysinfo.h>
#if defined(BSD)
#include <sys/sysctl.h>
#endif

#else
#error "Unable to define getMemorySize( ) for an unknown OS."
#endif

using namespace std;

namespace util
{
	/** Utility methods for memory info acquisition. */
	class MemoryInfo
	{
	public:
		/** @returns the total size of installed physical memory (RAM) in bytes. */
		static size_t getMemorySize();
		
		/** @returns the size of available free physical memory (RAM) in bytes. */
		static size_t getAvailableMemorySize();
	};
	
	inline size_t MemoryInfo::getAvailableMemorySize( )
	{
		struct sysinfo info;
		sysinfo( &info );
		return info.freeram;
	}
}

#endif