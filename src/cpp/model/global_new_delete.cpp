// optional_ops.cpp

#include "TbbAllocator.h"

using namespace model;

void * operator new( std::size_t size ) throw( std::bad_alloc )
{
	if( size == 0 ) size = 1;
    if( void* ptr = scalable_malloc( size ) )
	{
		AllocStatistics::notifyAlloc( scalable_msize( ptr ) );
        return ptr;
	}
    throw std::bad_alloc();
}

void * operator new( std::size_t size, const std::nothrow_t& ) throw()
{
	if( size == 0 ) size = 1;
    if( void* ptr = scalable_malloc( size ) )
	{
		AllocStatistics::notifyAlloc( scalable_msize( ptr ) );
        return ptr;
	}
    return NULL;
}

void *operator new[]( std::size_t size ) throw( std::bad_alloc )
{
	return operator new( size );
}

void *operator new[]( std::size_t size, const std::nothrow_t& ) throw()
{
	return operator new( size, std::nothrow );
}

void operator delete( void* ptr ) throw()
{
	if( ptr != 0 )
	{
		AllocStatistics::notifyDealloc( scalable_msize( ptr ) );
		scalable_free( ptr );
	}
}

void operator delete( void* ptr, const std::nothrow_t& ) throw()
{
	if( ptr != 0 )
	{
		AllocStatistics::notifyDealloc( scalable_msize( ptr ) );
		scalable_free( ptr );
	}
}

void operator delete[]( void* ptr ) throw()
{
	operator delete( ptr );
}

void operator delete[]( void* ptr, const std::nothrow_t& ) throw()
{
	operator delete( ptr, std::nothrow );
}