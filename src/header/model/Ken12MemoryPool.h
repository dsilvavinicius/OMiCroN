#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#include <cstdlib>
#include <iostream>
#include <mutex>
#include "BasicTypes.h"
#include "IMemoryPool.h"

using namespace std;

namespace model
{
	/** Thread-safe template implementation of Ben Kenwright's Fast Efficient Fixed-Sized Memory Pool paper:
	 * http://www.thinkmind.org/index.php?view=article&articleid=computation_tools_2012_1_10_80006. */
	template< typename T >
	class Ken12MemoryPool
	: public IMemoryPool< T >
	{
		uint	m_numOfBlocks;		// Num of blocks
		uint	m_numFreeBlocks;	// Num of remaining blocks
		uint	m_numInitialized;	// Num of initialized blocks
		T*		m_memStart;			// Beginning of memory pool
		T*		m_next;				// Num of next free block
		mutex	poolLock;			// Lock to the pool
		
		public:

		Ken12MemoryPool()
		{
			m_numOfBlocks = 0;
			m_numFreeBlocks = 0;
			m_numInitialized = 0;
			m_memStart = NULL;
			m_next = 0;
		}
		
		~Ken12MemoryPool()
		{
			destroyPool();
		}
		
		void createPool( uint numOfBlocks )
		{
			destroyPool();
			
			if( numOfBlocks > 0 )
			{
				m_numOfBlocks = numOfBlocks;
				m_memStart = reinterpret_cast< T* >( new uchar[ sizeof( T ) * m_numOfBlocks ] );
				m_numFreeBlocks = numOfBlocks;
				m_next = m_memStart;
			}
		}
		
		T* allocate() override
		{
			lock_guard< mutex > guard( poolLock );
			if( m_numInitialized < m_numOfBlocks )
			{
				uint* p = ( uint* ) addrFromIndex( m_numInitialized );
				*p = m_numInitialized + 1;
				m_numInitialized++;
			}
			T* ret = NULL;
			if( m_numFreeBlocks > 0 )
			{
				ret = m_next;
				--m_numFreeBlocks;
				if( m_numFreeBlocks != 0 )
				{
					m_next = addrFromIndex( *( ( uint* ) m_next ) );
				}
				else
				{
					m_next = NULL;
				}
			}
			return ret;
		}
		
		/** UNSUPORTED. */
		T* allocateArray( const size_t& size ) override
		{
			throw logic_error( "MemoryPool::allocateArray() is unsuported." );
		}
		
		void deallocate( T* p ) override
		{
			lock_guard< mutex > guard( poolLock );
			if( m_next != NULL )
			{
				( *( uint* )p ) = indexFromAddr( m_next );
				m_next = p;
			}
			else
			{
				*( ( uint* )p ) = m_numOfBlocks;
				m_next = p;
			}
			++m_numFreeBlocks;
		}
		
		void deallocateArray( T* p ) override
		{
			throw logic_error( "MemoryPool::deallocateArray() is unsuported." );
		}
		
		size_t usedBlocks() const override
		{
			return m_numOfBlocks - m_numFreeBlocks;
		}
		
		size_t memoryUsage() const override;
		
		uint getNumFreeBlocks() const
		{
			return m_numFreeBlocks;
		}
		
		float getFreeBlockPercentage() const
		{
			return  ( float ) m_numFreeBlocks / ( float ) m_numOfBlocks;
		}
		
		uint getNumBlocks() const
		{
			return m_numOfBlocks;
		}
		
		friend ostream& operator<<( ostream& out, const Ken12MemoryPool& pool )
		{
			out << "num blocks: " << pool.m_numOfBlocks << endl
				<< "block size: " << pool.m_sizeOfEachBlock << endl
				<< "free: " << pool.m_numFreeBlocks << endl
				<< "initialized: " << pool.m_numInitialized << endl
				<< "mem start: 0x" << hex << ( uint* ) pool.m_memStart << dec << endl
				<< "next: 0x" << hex << ( uint* ) pool.m_next << dec << endl;
			
			if( pool.m_next )
			{
				out << "next index: " << *( ( uint* ) pool.m_next ) << endl;
			}
			
			return out;
		}
		
	private:
		void destroyPool()
		{
			lock_guard< mutex > guard( poolLock );
			
			m_numOfBlocks = 0;
			m_numFreeBlocks = 0;
			m_numInitialized = 0;
			m_next = 0;
			
			if( m_memStart != NULL )
			{
				delete[] ( uchar* ) m_memStart;
				m_memStart = NULL;
			}
		}
		
		T* addrFromIndex( uint i ) const
		{
			return m_memStart + i;
		}
		
		uint indexFromAddr( const T* p ) const
		{
			return ( uint )( p - m_memStart );
		}
	};
	
	template< typename T >
	size_t Ken12MemoryPool< T >::memoryUsage() const
	{
		return usedBlocks() * sizeof( T );
	}
}

#endif