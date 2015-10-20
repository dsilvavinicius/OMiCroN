#ifndef FREE_LIST_POOL
#define FREE_LIST_POOL
#include "IMemoryPool.h"

namespace model
{
	template< typename T >
	class FreeListPool
	: public IMemoryPool< T >
	{
	public:
		FreeListPool( const size_t& chunkSize =  );
		
		~FreeListPool();
		
		T* allocate() override;
		
		T* allocateArray( const size_t& size ) override;
		
		void deallocate( T* p ) override;
		
		void deallocateArray( T* p ) override;
		
		size_t memoryUsage() const override;
		
		size_t usedBlocks() const override;
	
	private:
		void newChunk();
		
		vector< T* > m_chunks;
		size_t m_chunkSize;
		size_t m_usedBlocks;
		T** m_freeList;
	};

	template< typename T >
	FreeListPool< T >::~FreeListPool()
	{
		assert( m_usedBlocks == 0);

		for( T* chunk : m_chunks )
		{
			free( chunk );
		}
	}

	

	
}

#endif