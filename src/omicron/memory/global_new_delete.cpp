// optional_ops.cpp

#include "omicron/memory/tbb_allocator.h"

using namespace omicron::memory;

void * operator new( std::size_t size )
{
	if( size == 0 ) size = 1;
    if( void* ptr = scalable_malloc( size ) )
	{
		AllocStatistics::notifyAlloc( scalable_msize( ptr ) );
        return ptr;
	}
    throw std::bad_alloc();
}

void * operator new( std::size_t size, const std::nothrow_t& )
{
	if( size == 0 ) size = 1;
    if( void* ptr = scalable_malloc( size ) )
	{
		AllocStatistics::notifyAlloc( scalable_msize( ptr ) );
        return ptr;
	}
    return NULL;
}

void *operator new[]( std::size_t size )
{
	return operator new( size );
}

void *operator new[]( std::size_t size, const std::nothrow_t& )
{
	return operator new( size, std::nothrow );
}

void operator delete( void* ptr )
{
	if( ptr != 0 )
	{
		AllocStatistics::notifyDealloc( scalable_msize( ptr ) );
		scalable_free( ptr );
	}
}

void operator delete( void* ptr, const std::nothrow_t& )
{
	if( ptr != 0 )
	{
		AllocStatistics::notifyDealloc( scalable_msize( ptr ) );
		scalable_free( ptr );
	}
}

void operator delete[]( void* ptr )
{
	operator delete( ptr );
}

void operator delete[]( void* ptr, const std::nothrow_t& )
{
	operator delete( ptr, std::nothrow );
}
