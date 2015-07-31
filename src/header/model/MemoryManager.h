#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include "MemoryPool.h"
#include <vector>

namespace model
{
	/** Utilizes Memory pools in order to manage creation of MortonCode, Point and Node objects. Allocates one large
	 * memory chunk to be served for each type. Reuses deallocated memory for next allocations. Also provides API to
	 * the number of current available memory blocks for each type. */
	class MemoryManager
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
		
		/** Gets the MemoryManager singleton instance. */
		static MemoryManager& instance();
		
		/** Allocates memory for a managed type. */
		void* allocate( const MEMORY_POOL_TYPE& type );
		
		/** Deallocates memory for a managed type. */
		void deallocate( const MEMORY_POOL_TYPE& type, void* p );
		
		/** Returns the number of blocks for a given managed type. */
		uint numBlocks( const MEMORY_POOL_TYPE& type ) const;
		
		/** Gets the number of yet available memory blocks of a managed type. */
		uint freeBlocks( const MEMORY_POOL_TYPE& type ) const;
		
		/** Gets the percentage of yet available memory blocks of a managed type. */
		float freeBlocksPercentage( const MEMORY_POOL_TYPE& type ) const;
		
		/** Verifies if all initialized MemoryPools' percentage of free blocks are above the passed percentage
		 * threshold. */
		bool hasEnoughMemory( const float& percentageThreshold ) const;
		
		friend ostream& operator<<( ostream& out, const MemoryManager& manager );
		
	private:
		/** Ctor doesn't allocates memory. Use initInstance to initialize it. */
		MemoryManager() {}
		
		/** Initializes instance with the number of memory blocks for each type specified by the parameters.
		 * @param nShallowMorton is the number of ShallowMortonCode memory blocks.
		 * @param nMediumMorton is the number of MediumMortonCode memory blocks.
		 * @param nPoints is the number of Point memory blocks.
		 * @param nExtendedPoints is the number of ExtendedPoint memory blocks.
		 * @param nNodes is the number of LeafNode or InnerNode memory blocks. */
		void init( const ulong& nShallowMorton, const ulong& nMediumMorton, const ulong& nPoints,
				   const ulong& nExtendedPoints, const ulong& nNodes );
		
		MemoryPool m_pools[ MEMORY_POOL_TYPE::COUNT ]; // MemoryPools. One for each managed type.
		static uint SIZES[ MEMORY_POOL_TYPE::COUNT ]; // Sizes of the managed types.
		
		static MemoryManager m_instance;
	};
	
	inline void MemoryManager::initInstance( const ulong& nShallowMorton, const ulong& nMediumMorton,
											 const ulong& nPoints, const ulong& nExtendedPoints, const ulong& nNodes )
	{
		m_instance.init( nShallowMorton, nMediumMorton, nPoints, nExtendedPoints, nNodes );
	}
	
	inline MemoryManager& MemoryManager::instance()
	{
		return m_instance;
	}

	inline void* MemoryManager::allocate( const MEMORY_POOL_TYPE& type )
	{
		//cout << "Alloc " << type << endl;
		void* mem = m_pools[ type ].allocate();
		//cout << "Free: " << freeBlocks( type ) << endl << endl;
		return mem;
	}
	
	inline void MemoryManager::deallocate( const MEMORY_POOL_TYPE& type, void* p )
	{
		//cout << "Dealloc " << type << endl;
		m_pools[ type ].deAllocate( p );
		//cout << "Free: " << freeBlocks( type ) << endl << endl;
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
	
	inline ostream& operator<<( ostream& out, const MemoryManager& manager )
	{
		out << "Free 4 blocks (Shallow Code): " << manager.freeBlocks( MemoryManager::FOUR_BYTES ) << ", "
			<< manager.freeBlocksPercentage( MemoryManager::FOUR_BYTES ) << " fraction" << endl
			<< "Free 8 blocks (Medium Code): " << manager.freeBlocks( MemoryManager::EIGHT_BYTES ) << ", "
			<< manager.freeBlocksPercentage( MemoryManager::EIGHT_BYTES ) << " fraction" << endl
			<< "Free 24 blocks (Point): " << manager.freeBlocks( MemoryManager::TWENTY_FOUR_BYTES ) << ", "
			<< manager.freeBlocksPercentage( MemoryManager::TWENTY_FOUR_BYTES ) << " fraction" << endl
			<< "Free 32 blocks (Node): " << manager.freeBlocks( MemoryManager::THIRTY_TWO ) << ", "
			<< manager.freeBlocksPercentage( MemoryManager::THIRTY_TWO ) << " fraction" << endl
			<< "Free 36 blocks (Extended Point): " << manager.freeBlocks( MemoryManager::THIRTY_SIX ) << ", "
			<< manager.freeBlocksPercentage( MemoryManager::THIRTY_SIX ) << " fraction" << endl;
		return out;
	}
	
	inline void MemoryManager::init( const ulong& nShallowMorton, const ulong& nMediumMorton, const ulong& nPoints,
									 const ulong& nExtendedPoints, const ulong& nNodes )
	{
		m_pools[ FOUR_BYTES ].createPool( SIZES[ FOUR_BYTES ], nShallowMorton );
		m_pools[ EIGHT_BYTES ].createPool( SIZES[ EIGHT_BYTES ], nMediumMorton );
		m_pools[ TWENTY_FOUR_BYTES ].createPool( SIZES[ TWENTY_FOUR_BYTES ], nPoints );
		m_pools[ THIRTY_TWO ].createPool( SIZES[ THIRTY_TWO ], nNodes );
		m_pools[ THIRTY_SIX ].createPool( SIZES[ THIRTY_SIX ], nExtendedPoints );
	}
}

#endif