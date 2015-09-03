#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <vector>
#include <sstream>
#include "MemoryPool.h"
#include "IMemoryManager.h"

namespace model
{
	/** Utilizes Memory pools in order to manage creation of MortonCode, Point and Node objects. Allocates one large
	 * memory chunk to be served for each type. Reuses deallocated memory for next allocations. Also provides API to
	 * the number of current available memory blocks for each type. */
	class MemoryManager
	: public SingletonMemoryManager
	{
	public:
		enum MEMORY_POOL_TYPE
		{
			FOUR_BYTES,			// 4 bytes block.
			EIGHT_BYTES,		// 8 bytes block.
			TWENTY_FOUR_BYTES,	// 24 bytes block.
			THIRTY_TWO,			// 32 bytes block.
			THIRTY_SIX,			// 36 bytes block.
			COUNT
		};
		
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
		
		void* allocate( const size_t& size ) override;
		
		void deallocate( void* p ) override;
		
		string toString() const override;
		
		bool hasEnoughMemory( const float& percentageThreshold ) const override;
		
		/** Returns the number of blocks for a given managed type. */
		uint numBlocks( const MEMORY_POOL_TYPE& type ) const;
		
		/** Gets the number of yet available memory blocks of a managed type. */
		uint freeBlocks( const MEMORY_POOL_TYPE& type ) const;
		
		/** Gets the percentage of yet available memory blocks of a managed type. */
		float freeBlocksPercentage( const MEMORY_POOL_TYPE& type ) const;
		
	private:
		/** Initializes instance with the number of memory blocks for each type specified by the parameters.
		 * @param nShallowMorton is the number of ShallowMortonCode memory blocks.
		 * @param nMediumMorton is the number of MediumMortonCode memory blocks.
		 * @param nPoints is the number of Point memory blocks.
		 * @param nExtendedPoints is the number of ExtendedPoint memory blocks.
		 * @param nNodes is the number of LeafNode or InnerNode memory blocks. */
		MemoryManager( const ulong& nShallowMorton, const ulong& nMediumMorton, const ulong& nPoints,
					   const ulong& nExtendedPoints, const ulong& nNodes );
		
		MemoryPool m_pools[ MEMORY_POOL_TYPE::COUNT ]; // MemoryPools. One for each managed type.
		static uint M_SIZES[ MEMORY_POOL_TYPE::COUNT ]; // Sizes of the managed types.
	};
	
	inline void MemoryManager::initInstance( const ulong& nShallowMorton, const ulong& nMediumMorton,
											 const ulong& nPoints, const ulong& nExtendedPoints, const ulong& nNodes )
	{
		if( m_instance != nullptr )
		{
			deleteInstance();
		}
		m_instance = new MemoryManager( nShallowMorton, nMediumMorton, nPoints, nExtendedPoints, nNodes );
	}

	inline void* MemoryManager::allocate( const size_t& size )
	{
		for( int poolType = 0; poolType < MEMORY_POOL_TYPE::COUNT; ++poolType )
		{
			if( M_SIZES[ poolType ] == size )
			{
				return m_pools[ poolType ].allocate();
			}
		}
		
		assert( false && "Cannot allocate requested bytes." );
	}
	
	inline void MemoryManager::deallocate( void* p )
	{
		uchar* charP = static_cast< uchar* >( p );
		//cout << "Dealloc " << type << endl << endl;
		for( int poolType = 0; poolType < MEMORY_POOL_TYPE::COUNT; ++poolType )
		{
			if( m_pools[ poolType ].isPointerIn( charP ) )
			{
				m_pools[ poolType ].deAllocate( charP );
				return;
			}
		}
		
		assert( false && "Cannot deallocate p." );
		//cout << *this << endl;
	}
	
	inline string MemoryManager::toString() const
	{
		stringstream ss;
		ss	<< "=== MemManager ===" << endl << endl
			<< "Free 4 blocks (Shallow Code): " << freeBlocks( MemoryManager::FOUR_BYTES ) << ", "
			<< freeBlocksPercentage( MemoryManager::FOUR_BYTES ) << " fraction" << endl << endl
			<< "Free 8 blocks (Medium Code): " << freeBlocks( MemoryManager::EIGHT_BYTES ) << ", "
			<< freeBlocksPercentage( MemoryManager::EIGHT_BYTES ) << " fraction" << endl << endl
			<< "Free 24 blocks (Point): " << freeBlocks( MemoryManager::TWENTY_FOUR_BYTES ) << ", "
			<< freeBlocksPercentage( MemoryManager::TWENTY_FOUR_BYTES ) << " fraction" << endl << endl
			<< "Free 32 blocks (Node): " << freeBlocks( MemoryManager::THIRTY_TWO ) << ", "
			<< freeBlocksPercentage( MemoryManager::THIRTY_TWO ) << " fraction" << endl << endl
			<< "Free 36 blocks (Extended Point): " << freeBlocks( MemoryManager::THIRTY_SIX ) << ", "
			<< freeBlocksPercentage( MemoryManager::THIRTY_SIX ) << " fraction" << endl << endl
			<< "=== End of MemManager ===" << endl;
	}
	
	inline bool MemoryManager::hasEnoughMemory( const float& percentageThreshold ) const
	{
		bool hasMemory = true;
		
		for( int memPoolType = FOUR_BYTES; memPoolType < COUNT; ++memPoolType )
		{
			if( m_pools[ memPoolType ].getNumBlocks() )
			{
				if( m_pools[ memPoolType ].getFreeBlockPercentage() < percentageThreshold )
				{
					return false;
				}
			}
		}
		
		return hasMemory;
	}
	
	inline uint MemoryManager::numBlocks( const MEMORY_POOL_TYPE& type ) const
	{
		return m_pools[ type ].getNumBlocks();
	}
	
	inline uint MemoryManager::freeBlocks( const MEMORY_POOL_TYPE& type ) const
	{
		return m_pools[ type ].getNumFreeBlocks();
	}
	
	inline float MemoryManager::freeBlocksPercentage( const MEMORY_POOL_TYPE& type ) const
	{
		return m_pools[ type ].getFreeBlockPercentage();
	}
	
	inline MemoryManager::MemoryManager( const ulong& nShallowMorton, const ulong& nMediumMorton, const ulong& nPoints,
										 const ulong& nExtendedPoints, const ulong& nNodes )
	{
		m_pools[ FOUR_BYTES ].createPool( M_SIZES[ FOUR_BYTES ], nShallowMorton );
		m_pools[ EIGHT_BYTES ].createPool( M_SIZES[ EIGHT_BYTES ], nMediumMorton );
		m_pools[ TWENTY_FOUR_BYTES ].createPool( M_SIZES[ TWENTY_FOUR_BYTES ], nPoints );
		m_pools[ THIRTY_TWO ].createPool( M_SIZES[ THIRTY_TWO ], nNodes );
		m_pools[ THIRTY_SIX ].createPool( M_SIZES[ THIRTY_SIX ], nExtendedPoints );
	}
}

#endif