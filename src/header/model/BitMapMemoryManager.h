#ifndef BITMAP_MEMORY_MANAGER_H
#define BITMAP_MEMORY_MANAGER_H

#include "IMemoryManager.h"
#include "BitMapMemoryPool.h"

namespace model
{
	class BitMapMemoryManager
	: public SingletonMemoryManager 
	{
	public: 
		BitMapMemoryManager(){}
		~BitMapMemoryManager( ) {}
		void* allocate( const size_t& size ) override;
		void deallocate( void* p ) override;
		bool hasEnoughMemory( const float& percentageThreshold ) const override;
		string toString() const override;
	
	private:
		vector< BitMapMemoryPool > m_pools; // Each pool serves requests of a given size.
		int m_maxObjectSize; // Size of the maximum object size of all pools already created.
	};
}

#endif