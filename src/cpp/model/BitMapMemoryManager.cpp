#include <sstream>
#include "BitMapMemoryManager.h"

namespace model
{
	void* BitMapMemoryManager::allocate( const size_t& size )
	{
		if( size < m_maxObjectSize )
		{
			// Non-array version.
			for( BitMapMemoryPool pool : m_pools )
			{
				if( pool.getObjectSize() == size )
				{
					return pool.allocate();
				}
			}
			
			// Create a new pool for this request size
			BitMapMemoryPool pool( size );
			m_pools.push_back( pool );
			return pool.allocate();
		}
		else
		{
			// Array version.
			for( BitMapMemoryPool pool : m_pools )
			{
				if( pool.getObjectSize() % size == 0 )
				{
					return pool.allocateArray( size );
				}
			}
			
			stringstream ss;
			ss << "Cannot allocate array with size " << size;
			throw logic_error( ss.str() );
		}
	}
	
	void BitMapMemoryManager::deallocate( void* p )
	{
		bool deallocated = false;
		for( BitMapMemoryPool pool : m_pools )
		{
			if( pool.deallocate( p ) )
			{
				deallocated = true;
				break;
			}
		}
		
		if( !deallocated )
		{
			throw logic_error( "Couldn't deallocate memory." );
		}
	}

	bool BitMapMemoryManager::hasEnoughMemory(const float& percentageThreshold) const
	{
		
	}
	
	string BitMapMemoryManager::toString() const
	{
		
	}
}