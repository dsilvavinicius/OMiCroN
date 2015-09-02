#ifndef IMEMORY_MANAGER_H
#define IMEMORY_MANAGER_H

#include <cstdlib>

using namespace std;

namespace model
{
	/** Interface for MemoryManagers. */
	class IMemoryManager
	{
		/** Allocates memory for a managed type. */
		virtual void* allocate( const size_t& size ) = 0;
		
		/** Deallocates memory for a managed type. */
		virtual void deallocate( void* p ) = 0;
		
		/** Verifies if the free memory is above the passed percentage threshold. */
		virtual bool hasEnoughMemory( const float& percentageThreshold ) const = 0;
	};
}

#endif