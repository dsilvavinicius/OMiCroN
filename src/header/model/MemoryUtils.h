#ifndef MEMORY_UTILS_H
#define MEMORY_UTILS_H

#include <memory.h>
#include "ManagedAllocator.h"

using namespace std;

namespace model
{
	/** Allocate and constructs the type T and an associated shared_ptr in managed memory, returning a shared_ptr for T.
	 * Use this method if the objecte and its pointer should stay in managed memory. */
	template< class T, class... Args >
	inline shared_ptr< T > makeManaged( Args&&... args )
	{
		return allocate_shared< T >( ManagedAllocator< T >(), std::forward< Args >( args ) ... );
	}
}

#endif