#ifndef BITMAP_ALLOCATOR_H
#define BITMAP_ALLOCATOR_H

#include "IMemoryManager.h"
#include <qabstractitemmodel.h>

namespace model
{
	template< typename T >
	class DefaultManagedAllocator
	{
	public:
		using value_type = T;
		using pointer = T*;
		using const_pointer = const pointer;
		using reference = T&;
		using const_reference = const reference;
		using size_type = size_t;
		using difference_type = ptrdiff_t;
		
		template< typename U >
		struct rebind{ using other = ManagedAllocator< U >; };
		
		DefaultManagedAllocator(){}
		DefaultManagedAllocator( const ManagedAllocator< T >& ){}
		
		template< typename U >
		DefaultManagedAllocator( const ManagedAllocator< U >& ){}
		
		pointer allocate( size_type n );
		void deallocate( pointer p, size_type n );
		
		template< typename U >
		void destroy( U* p );
		
		bool operator==( const DefaultManagedAllocator& ){ return true; }
		bool operator!=( const DefaultManagedAllocator& ){ return false; }
	};
	
	/** Defines allocator types for Morton, Point, Inner and Leaf types. */
	template< typename Morton, typename Point, typename Node >
	struct ManagedAllocGroup
	{
		using MortonAlloc = ManagedAllocator< Morton >;
		using PointAlloc = ManagedAllocator< Point >;
		using NodeAlloc = ManagedAllocator< Node >;
	};
	
	template< typename T >
	inline T* DefaultManagedAllocator< T >::allocate( size_type n )
	{
		IMemoryManager& manager = SingletonMemoryManager::instance();
		return  n == 1 ? manager.alloc< T >() : manager.allocArray< T >(  n * sizeof( T ) );
	}

	template< typename T >
	inline void DefaultManagedAllocator< T >::deallocate( pointer p, size_type n )
	{
		IMemoryManager& manager = SingletonMemoryManager::instance();
		n == 1 ? manager.dealloc< T >( p ) : manager.deallocArray< T >( p );
	}
	
	template< typename T >
	template< typename U >
	inline void DefaultManagedAllocator< T >::destroy( U* p )
	{
		p->~U();
	}
}

#endif