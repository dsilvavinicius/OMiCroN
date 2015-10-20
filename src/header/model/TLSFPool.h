#ifndef TLSF_POOL_H
#define TLSF_POOL_H

#include <cassert>
#include <cstdlib>
#include <stdexcept>
#include <mutex>
#include <tlsf.h>
#include "IMemoryPool.h"

using namespace std;

namespace model
{
	/** Uses implementation of "A constant-time dynamic storage allocator for real-time systems" paper detailed in 
	 * http://www.gii.upv.es/tlsf/ for dynamic size allocations and a free lists pool allocator detailed in
	 * http://www.gamedev.net/page/resources/_/technical/general-programming/c-custom-memory-allocation-r3010 for fixed
	 * size allocations. */
	template< typename T >
	class TLSFPool
	: public IMemoryPool< T >
	{
	public:
		TLSFPool( const size_t& poolSize = 1024 * 1024 * 10 );
		~TLSFPool();
		
		T* allocate() override;
		T* allocateArray( const size_t& size ) override;
		void deallocate( T* p ) override;
		void deallocateArray( T* p ) override;
		size_t memoryUsage() const override;
		size_t usedBlocks() const override;
	
	private:
		void newFreeListChunk();
		
		// TLSF related members.
		size_t m_poolSize;
		void* m_rawPool;
		mutex m_tlsfLock;
		
		// Free lists related members.
		vector< T* > m_freeListChunks;
		size_t m_freeListUsedBlocks;
		T** m_freeList;
		mutex m_freeListLock;
	};
	
	template< typename T >
	TLSFPool< T >::TLSFPool( const size_t& poolSize )
	: m_poolSize( poolSize ),
	m_freeListUsedBlocks( 0 ),
	m_freeList( nullptr )
	{
		//cout << "sizeof( T ) = " << sizeof( T ) << " sizeof( T* ) = " << sizeof( T* ) << endl << endl;
		//assert( sizeof( T ) >= sizeof( T* ) ); // assertion needed for free lists.
		
		size_t nBytes = sizeof( T ) * poolSize;
		m_rawPool = malloc( nBytes );
		init_memory_pool( nBytes, m_rawPool );
	}
	
	template< typename T >
	TLSFPool< T >::~TLSFPool()
	{
		destroy_memory_pool( m_rawPool );
		free( m_rawPool );
		
		assert( m_freeListUsedBlocks == 0 );
		
		for( T* chunk : m_freeListChunks )
		{
			free( chunk );
		}
	}
	
	template< typename T >
	T* TLSFPool< T >::allocate()
	{
		lock_guard< mutex > guard( m_freeListLock );
		
		if( m_freeList == nullptr )
		{
			newFreeListChunk();
		}

		T* p = ( T* ) m_freeList;

		m_freeList = ( T** )( *m_freeList );

		++m_freeListUsedBlocks;

		return p;
	}

	template< typename T >
	void TLSFPool< T >::deallocate( T* p )
	{
		lock_guard< mutex > guard( m_freeListLock );
		
		*( ( T** ) p ) = ( T* ) m_freeList;

		m_freeList = ( T** ) p;

		--m_freeListUsedBlocks;
	}
	
	template< typename T >
	inline T* TLSFPool< T >::allocateArray( const size_t& size )
	{
		lock_guard< mutex > guard( m_tlsfLock );
		
		//cout << "Alloc " << size << " bytes" << endl << endl;
		void* addr = malloc_ex( size, m_rawPool );
		assert( addr && "Cannot allocate array." );
		return reinterpret_cast< T* >( addr );
	}
	
	template< typename T >
	inline void TLSFPool< T >::deallocateArray( T* p )
	{
		lock_guard< mutex > guard( m_tlsfLock );
		free_ex( p, m_rawPool );
	}
	
	/** Statistics for used blocks are not available for the TLSF algorithm. Use memoryUsage() instead. */
	template< typename T >
	inline size_t TLSFPool< T >::usedBlocks() const
	{
		return 0;
	}
	
	template< typename T >
	inline size_t TLSFPool< T >::memoryUsage() const
	{
		return get_used_size( m_rawPool ) + m_freeListUsedBlocks * sizeof( T );
	}
	
	/** Creates a new chunk and prepend it into current free list. */
	template< typename T >
	void TLSFPool< T >::newFreeListChunk()
	{
		T* chunk = ( T* ) malloc( m_poolSize * sizeof( T ) );
		m_freeListChunks.push_back( chunk );
		
		T** prevFreeList = m_freeList;
		m_freeList = ( T** ) m_freeListChunks[ m_freeListChunks.size() - 1 ];
		T** p = m_freeList;

		//Initialize free blocks list
		for( size_t i = 0; i < m_poolSize - 1; ++i )
		{
			*p = ( ( T* ) p ) + 1;
			p = ( T** ) *p;
		}

		*p = ( T* ) prevFreeList;
	}
}

#endif