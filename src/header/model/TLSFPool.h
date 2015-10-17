#ifndef TLSF_POOL_H
#define TLSF_POOL_H

#include <cassert>
#include <cstdlib>
#include <stdexcept>
#include <tlsf.h>
#include "IMemoryPool.h"

using namespace std;

namespace model
{
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
		size_t m_poolSize;
		size_t m_usedMemory;
		tlsf_t m_tlsf;
	};
	
	template< typename T >
	TLSFPool< T >::TLSFPool( const size_t& poolSize )
	: m_poolSize( poolSize ),
	m_usedMemory( 0 )
	{
		size_t nBytes = sizeof( T ) * poolSize;
		void* mem = malloc( nBytes );
		m_tlsf = tlsf_create_with_pool( mem, nBytes );
	}
	
	template< typename T >
	TLSFPool< T >::~TLSFPool()
	{
		tlsf_destroy( m_tlsf );
	}
	
	template< typename T >
	inline T* TLSFPool< T >::allocate()
	{
		allocateArray( sizeof( T ) );
	}
	
	template< typename T >
	inline T* TLSFPool< T >::allocateArray( const size_t& size )
	{
		void* addr = tlsf_malloc( m_tlsf, size );
		if( !addr )
		{
			size_t nBytes = sizeof( T ) * m_poolSize;
			tlsf_add_pool( m_tlsf, malloc( nBytes ), nBytes );
			addr = tlsf_malloc( m_tlsf, size );
		}
		assert( addr && "Cannot allocate array." );
		m_usedMemory += tlsf_block_size( addr );
		return reinterpret_cast< T* >( addr );
	}
	
	template< typename T >
	inline void TLSFPool< T >::deallocate( T* p )
	{
		deallocateArray( p );
	}
	
	template< typename T >
	inline void TLSFPool< T >::deallocateArray( T* p )
	{
		m_usedMemory -= tlsf_block_size( p );
		tlsf_free( m_tlsf, p );
	}
	
	template< typename T >
	inline size_t TLSFPool< T >::usedBlocks() const
	{
		// Statistic not available.
		return 0;
	}
	
	template< typename T >
	inline size_t TLSFPool< T >::memoryUsage() const
	{
		return m_usedMemory;
	}
}

#endif