#ifndef MANAGED_ALLOCATOR_H
#define MANAGED_ALLOCATOR_H

#include "omicron/memory/tbb_allocator.h"

namespace omicron::memory
{
	template< typename T >
	using ManagedAllocator = TbbAllocator< T >;
}

#endif
