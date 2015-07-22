#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#include <cstdlib>
#include <iostream>
#include <mutex>
#include "BasicTypes.h"

using namespace std;

namespace model
{
	/** Thread-safe implementation of Ben Kenwright's Fast Efficient Fixed-Sized Memory Pool paper:
	 * http://www.thinkmind.org/index.php?view=article&articleid=computation_tools_2012_1_10_80006. */
	class MemoryPool
	{
		uint	m_numOfBlocks;		// Num of blocks
		uint	m_sizeOfEachBlock; 	// Size of each block
		uint	m_numFreeBlocks;	// Num of remaining blocks
		uint	m_numInitialized;	// Num of initialized blocks
		uchar*	m_memStart;			// Beginning of memory pool
		uchar*	m_next;				// Num of next free block
		mutex	poolLock;			// Lock to the pool
		
		public:

		MemoryPool()
		{
			m_numOfBlocks = 0;
			m_sizeOfEachBlock = 0;
			m_numFreeBlocks = 0;
			m_numInitialized = 0;
			m_memStart = NULL;
			m_next = 0;
		}
		
		~MemoryPool()
		{
			destroyPool();
		}
		
		void createPool( size_t sizeOfEachBlock, uint numOfBlocks )
		{
			m_numOfBlocks = numOfBlocks;
			m_sizeOfEachBlock = sizeOfEachBlock;
			
			if( m_memStart )
			{
				destroyPool();
			}
			
			m_memStart = new uchar[ m_sizeOfEachBlock * m_numOfBlocks ];
			m_numFreeBlocks = numOfBlocks;
			m_next = m_memStart;
		}
		
		void* allocate()
		{
			lock_guard< mutex > guard( poolLock );
			if( m_numInitialized < m_numOfBlocks )
			{
				uint* p = ( uint* ) addrFromIndex( m_numInitialized );
				*p = m_numInitialized + 1;
				m_numInitialized++;
			}
			void* ret = NULL;
			if( m_numFreeBlocks > 0 )
			{
				ret = ( void* ) m_next;
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
		
		void deAllocate( void* p )
		{
			lock_guard< mutex > guard( poolLock );
			if( m_next != NULL )
			{
				( *( uint* )p ) = indexFromAddr( m_next );
				m_next = ( uchar* )p;
			}
			else
			{
				*( ( uint* )p ) = m_numOfBlocks;
				m_next = ( uchar* )p;
			}
			++m_numFreeBlocks;
		}
		
		uint getNumFreeBlocks() const
		{
			return m_numFreeBlocks;
		}
		
		float getFreeBlockPercentage() const
		{
			//cout << "free: " << m_numFreeBlocks << ", blocks: " << m_numOfBlocks << endl;
			return  ( float ) m_numFreeBlocks / ( float ) m_numOfBlocks;
		}
		
		uint getNumBlocks() const
		{
			return m_numOfBlocks;
		}
		
	private:
		void destroyPool()
		{
			lock_guard< mutex > guard( poolLock );
			delete[] m_memStart;
			m_memStart = NULL;
		}
		
		uchar* addrFromIndex( uint i ) const
		{
			return m_memStart + ( i * m_sizeOfEachBlock );
		}
		
		uint indexFromAddr( const uchar* p ) const
		{
			return ( ( ( uint )( p - m_memStart )) / m_sizeOfEachBlock );
		}
	};
}

#endif