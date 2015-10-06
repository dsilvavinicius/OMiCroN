#ifndef BITMAP_ALLOCATOR_H
#define BITMAP_ALLOCATOR_H

#include "IMemoryManager.h"
#include <qabstractitemmodel.h>

namespace model
{
	template< typename T >
	class BitMapAllocator
	{
	public:
		using value_type = T;
		using pointer = T*;
		using const_pointer = const pointer;
		using reference = T&;
		using const_reference = const reference;
		using size_type = size_t;
		
		template< typename U >
		struct rebind{ using other = BitMapAllocator< U >; };
		
		BitMapAllocator(){}
		BitMapAllocator( const BitMapAllocator< T >& ){}
		
		template< typename U >
		BitMapAllocator( const BitMapAllocator< U >& ){}
		
		pointer allocate( size_type n );
		void deallocate( pointer p, size_type n );
		
		template< typename U >
		void destroy( U* p );
		
		bool operator==( const BitMapAllocator& ){ return true; }
		bool operator!=( const BitMapAllocator& ){ return false; }
	};
	
	/** Defines allocator types for Morton, Point, Inner and Leaf types. */
	template< typename Morton, typename Point, typename Inner, typename Leaf >
	struct BitMapAllocGroup
	{
		using MortonAlloc = BitMapAllocator< Morton >;
		using PointAlloc = BitMapAllocator< Point >;
		using InnerAlloc = BitMapAllocator< Inner >;
		using LeafAlloc = BitMapAllocator< Leaf >;
	};
	
	template< typename T >
	inline T* BitMapAllocator< T >::allocate( size_type n )
	{
		IMemoryManager& manager = SingletonMemoryManager::instance();
		return  n == 1 ? manager.alloc< T >() : manager.allocArray< T >(  n * sizeof( T ) );
	}

	template< typename T >
	inline void BitMapAllocator< T >::deallocate( pointer p, size_type n )
	{
		IMemoryManager& manager = SingletonMemoryManager::instance();
		n == 1 ? manager.dealloc< T >( p ) : manager.deallocArray< T >( p );
	}
	
	template< typename T >
	template< typename U >
	inline void BitMapAllocator< T >::destroy( U* p )
	{
		p->~U();
	}
}

#endif