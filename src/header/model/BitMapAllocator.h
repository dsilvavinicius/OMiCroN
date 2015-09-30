#ifndef BITMAP_ALLOCATOR_H
#define BITMAP_ALLOCATOR_H
#include "IMemoryManager.h"

namespace model
{
	template< typename T >
	class BitMapAllocator
	{
	public:
		using value_type = T;
		using pointer = T*;
		using size_type = size_t;
		
		template< typename U >
		struct rebind{ using other = BitMapAllocator< U >; };
		
		pointer allocate( size_type n );
		void deallocate( pointer p, size_type n );
	};
	
	template< typename T >
	inline T* BitMapAllocator::allocate( size_type n )
	{
		IMemoryManager& manager = SingletonMemoryManager::instance();
		return  n == 1 ? manager.alloc< T >() : manager.allocArray< T >(  n * sizeof( Type ) );
	}

	template< typename T >
	inline T* BitMapAllocator::deallocate( pointer p, size_type n )
	{
		IMemoryManager& manager = SingletonMemoryManager::instance();
		n == 1 ? manager.dealloc< T >( p ) : manager.deallocArray< T >( p );
	}
}

#endif