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

#include <cstdlib>

using namespace std;

namespace util
{
	/** Utility methods for memory info acquisition. */
	class MemoryInfo
	{
	public:
		/** @returns the total size of installed physical memory ( RAM ) in bytes. */
		static size_t getMemorySize();
		
		/** @returns the size of available free physical memory ( RAM ) in bytes. Returns at max 4GB. */
		static size_t getAvailableMemorySize();
	
	private:
		/** Approximates available memory size by allocation trials (fast and is more accurate after releases).
		 * Returns at max 4GB. */
		static size_t getAvailMemByTrial();
		
		/** DEPRECATED: Use getAvailMemByTrial() instead. */
		static size_t getAvailMemByMemInfo();
	};
}

#endif
