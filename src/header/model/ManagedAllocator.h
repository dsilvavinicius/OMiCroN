#ifndef MANAGED_ALLOCATOR_H
#define MANAGED_ALLOCATOR_H

#include <DefaultManagedAllocator.h>
#include <TbbAllocator.h>

namespace model
{
	template< typename T >
	using ManagedAllocator = TbbAllocator< T >;
}

#endif