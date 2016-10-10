#ifndef MANAGED_ALLOCATOR_H
#define MANAGED_ALLOCATOR_H

#include <TbbAllocator.h>

namespace model
{
	template< typename T >
	using ManagedAllocator = TbbAllocator< T >;
}

#endif