#include "MemoryManager.h"

using namespace std;

/*
void* operator new( std::size_t s ) throw( std::bad_alloc )
{
	switch( s )
	{
		case :
		default:
		{
			throw bad_alloc( "Memory allocation size " << itoa() << " not supported." );
		}
	}
	model::MemoryManager::instance().allocate( s );
}

void* operator new[]( std::size_t s ) throw( std::bad_alloc )
{
	throw bad_alloc( "new[] not supported." );
}

void operator delete( void* p ) throw()
{
	model::MemoryManager::instance().deallocate( p );
}

void operator delete[]( void *p ) throw()
{
    throw bad_alloc( "delete[] not supported." );
}*/