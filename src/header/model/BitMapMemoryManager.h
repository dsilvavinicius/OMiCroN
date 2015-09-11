#ifndef BITMAP_MEMORY_MANAGER_H
#define BITMAP_MEMORY_MANAGER_H

#include "IMemoryManager.h"
#include "BitMapMemoryPool.h"
#include "Point.h"
#include "MortonCode.h"
#include "OctreeNode.h"

namespace model
{
	class BitMapMemoryManager
	: public SingletonMemoryManager 
	{
	public:
		static void initInstance( const size_t& maxAllowedMem );
		
		~BitMapMemoryManager() {}
		
		template< typename T >
		void* allocate( const size_t& size ) override;
		
		template< typename T >
		void deallocate( void* p ) override;
		
		bool hasEnoughMemory( const float& percentageThreshold ) const override;
		
		string toString() const override;
		
		size_t usedMemory() const;
		
		template< typename T > BitMapMemoryPool< T >& getPool();
	
	private:
		BitMapMemoryManager( const size_t& maxAllowedMem )
		: m_maxAllowedMem( maxAllowedMem ){}
		
		size_t m_maxAllowedMem;
		BitMapMemoryPool< ShallowMortonCode > m_shallowMortonPool;
		BitMapMemoryPool< MediumMortonCode > m_mediumMortonPool;
		BitMapMemoryPool< Point > m_pointPool;
		BitMapMemoryPool< ExtendedPoint > m_extendedPointPool;
		BitMapMemoryPool< ShallowLeafNode > m_nodePool; // All nodes require the same memory amount, since the contents are
														// a vector of smart pointers.
	};
}

#endif