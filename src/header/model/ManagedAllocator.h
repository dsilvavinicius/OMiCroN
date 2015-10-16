#ifndef BITMAP_ALLOCATOR_H
#define BITMAP_ALLOCATOR_H

#include "IMemoryManager.h"
#include <qabstractitemmodel.h>

namespace model
{
	template< typename T >
	class ManagedAllocator
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
		
		ManagedAllocator(){}
		ManagedAllocator( const ManagedAllocator< T >& ){}
		
		template< typename U >
		ManagedAllocator( const ManagedAllocator< U >& ){}
		
		pointer allocate( size_type n );
		void deallocate( pointer p, size_type n );
		
		template< typename U >
		void destroy( U* p );
		
		bool operator==( const ManagedAllocator& ){ return true; }
		bool operator!=( const ManagedAllocator& ){ return false; }
	};
	
	/** Defines allocator types for Morton, Point, Inner and Leaf types. */
	template< typename Morton, typename Point, typename Inner, typename Leaf >
	struct ManagedAllocGroup
	{
		using MortonAlloc = ManagedAllocator< Morton >;
		using PointAlloc = ManagedAllocator< Point >;
		using InnerAlloc = ManagedAllocator< Inner >;
		using LeafAlloc = ManagedAllocator< Leaf >;
	};
	
	template< typename T >
	inline T* ManagedAllocator< T >::allocate( size_type n )
	{
		IMemoryManager& manager = SingletonMemoryManager::instance();
		return  n == 1 ? manager.alloc< T >() : manager.allocArray< T >(  n * sizeof( T ) );
	}

	template< typename T >
	inline void ManagedAllocator< T >::deallocate( pointer p, size_type n )
	{
		IMemoryManager& manager = SingletonMemoryManager::instance();
		n == 1 ? manager.dealloc< T >( p ) : manager.deallocArray< T >( p );
	}
	
	template< typename T >
	template< typename U >
	inline void ManagedAllocator< T >::destroy( U* p )
	{
		p->~U();
	}
}

#endif