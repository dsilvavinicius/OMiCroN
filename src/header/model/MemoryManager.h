#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <vector>
#include <sstream>
#include "MemoryPool.h"
#include "IMemoryManager.h"

namespace model
{
	template< typename T >
	class MortonCode;
	using ShallowMortonCode = MortonCode< unsigned int >;
	using MediumMortonCode = MortonCode< unsigned long >;
	
	class Point;
	using PointVector = vector< shared_ptr< Point > >;
	class ExtendedPoint;
	
	template< typename MortonCode, typename Contents >
	class LeafNode;
	template< typename Contents >
	using ShallowLeafNode = LeafNode< ShallowMortonCode, Contents >;
	
	/** Utilizes Memory pools in order to manage creation of MortonCode, Point and Node objects. Allocates one large
	 * memory chunk to be served for each type. Reuses deallocated memory for next allocations. Also provides API to
	 * the number of current available memory blocks for each type. */
	class MemoryManager
	: public SingletonMemoryManager
	{
	public:
		/** Initializes the singleton instance with the number of memory blocks for each type specified by the
		 * parameters. If it is already initialized, early allocated memory is deleted and new allocations are done
		 * accordingly with parameters.
		 * @param nShallowMorton is the number of ShallowMortonCode memory blocks.
		 * @param nMediumMorton is the number of MediumMortonCode memory blocks.
		 * @param nPoints is the number of Point memory blocks.
		 * @param nExtendedPoints is the number of ExtendedPoint memory blocks.
		 * @param nNodes is the number of LeafNode or InnerNode memory blocks. */
		static void initInstance( const ulong& nShallowMorton, const ulong& nMediumMorton, const ulong& nPoints,
								  const ulong& nExtendedPoints, const ulong& nNodes );
		
		void* allocate( const MANAGED_TYPE_FLAG& type ) override;
		
		void* allocateArray(  const size_t& size, const MANAGED_TYPE_FLAG& type ) override
		{
			throw logic_error( "Array allocation is unsupported." );
		}
		
		void deallocate( void* p, const MANAGED_TYPE_FLAG& type ) override;
		
		void deallocateArray( void* p, const MANAGED_TYPE_FLAG& type ) override
		{
			throw logic_error( "Array deallocation is unsupported." );
		}
		
		size_t usedMemory() const override;
		
		size_t maxAllowedMem() const override;
		
		bool hasEnoughMemory( const float& percentageThreshold ) const override;
		
		string toString() const override;
		
		size_t numBlocks( const MANAGED_TYPE_FLAG& type ) const;
		
	private:
		/** Initializes instance with the number of memory blocks for each type specified by the parameters.
		 * @param nShallowMorton is the number of ShallowMortonCode memory blocks.
		 * @param nMediumMorton is the number of MediumMortonCode memory blocks.
		 * @param nPoints is the number of Point memory blocks.
		 * @param nExtendedPoints is the number of ExtendedPoint memory blocks.
		 * @param nNodes is the number of LeafNode or InnerNode memory blocks. */
		MemoryManager( const ulong& nShallowMorton, const ulong& nMediumMorton, const ulong& nPoints,
					   const ulong& nExtendedPoints, const ulong& nNodes );
		
		MemoryPool< ShallowMortonCode > m_shallowMortonPool;
		MemoryPool< MediumMortonCode > m_mediumMortonPool;
		MemoryPool< Point > m_pointPool;
		MemoryPool< ExtendedPoint > m_extendedPointPool;
		MemoryPool< ShallowLeafNode< PointVector > > m_nodePool;
		size_t m_maxAllowedMem;
	};
	
	inline void MemoryManager::initInstance( const ulong& nShallowMorton, const ulong& nMediumMorton,
											 const ulong& nPoints, const ulong& nExtendedPoints, const ulong& nNodes )
	{
		m_instance = unique_ptr< IMemoryManager >( new MemoryManager( nShallowMorton, nMediumMorton, nPoints,
																	  nExtendedPoints, nNodes ) );
	}
		
	inline size_t MemoryManager::maxAllowedMem() const
	{
		return m_maxAllowedMem;
	}
	
	inline bool MemoryManager::hasEnoughMemory( const float& percentageThreshold ) const
	{
		return ( m_maxAllowedMem - usedMemory() ) > float( m_maxAllowedMem * percentageThreshold );
	}
}

#endif