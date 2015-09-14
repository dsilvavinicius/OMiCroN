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
		
		string toString() const override;
		
		bool hasEnoughMemory( const float& percentageThreshold ) const override;
		
		/** Returns the number of blocks for a given managed type. */
		uint numBlocks( const MANAGED_TYPE_FLAG& type ) const;
		
		/** Gets the number of yet available memory blocks of a managed type. */
		uint freeBlocks( const MANAGED_TYPE_FLAG& type ) const;
		
		/** Gets the percentage of yet available memory blocks of a managed type. */
		float freeBlocksPercentage( const MANAGED_TYPE_FLAG& type ) const;
		
	private:
		/** Initializes instance with the number of memory blocks for each type specified by the parameters.
		 * @param nShallowMorton is the number of ShallowMortonCode memory blocks.
		 * @param nMediumMorton is the number of MediumMortonCode memory blocks.
		 * @param nPoints is the number of Point memory blocks.
		 * @param nExtendedPoints is the number of ExtendedPoint memory blocks.
		 * @param nNodes is the number of LeafNode or InnerNode memory blocks. */
		MemoryManager( const ulong& nShallowMorton, const ulong& nMediumMorton, const ulong& nPoints,
					   const ulong& nExtendedPoints, const ulong& nNodes );
		
		MemoryPool m_pools[ MANAGED_TYPE_FLAG::COUNT ]; // MemoryPools. One for each managed type.
		static uint M_SIZES[ MANAGED_TYPE_FLAG::COUNT ]; // Sizes of the managed types.
	};
	
	inline void MemoryManager::initInstance( const ulong& nShallowMorton, const ulong& nMediumMorton,
											 const ulong& nPoints, const ulong& nExtendedPoints, const ulong& nNodes )
	{
		m_instance = unique_ptr< IMemoryManager >( new MemoryManager( nShallowMorton, nMediumMorton, nPoints,
																	  nExtendedPoints, nNodes ) );
	}

	inline void* MemoryManager::allocate( const MANAGED_TYPE_FLAG& type )
	{
		return m_pools[ type ].allocate();
	}
	
	inline void MemoryManager::deallocate( void* p, const MANAGED_TYPE_FLAG& type )
	{
		m_pools[ type ].deAllocate( p );
	}
	
	inline string MemoryManager::toString() const
	{
		stringstream ss;
		ss	<< "=== MemManager ===" << endl << endl
			<< "Free Shallow Code blocks: " << freeBlocks( SHALLOW_MORTON ) << ", "
			<< freeBlocksPercentage( SHALLOW_MORTON ) << " fraction" << endl << endl
			<< "Free Medium Codeblocks: " << freeBlocks( MEDIUM_MORTON ) << ", "
			<< freeBlocksPercentage( MEDIUM_MORTON ) << " fraction" << endl << endl
			<< "Free Point blocks: " << freeBlocks( POINT ) << ", "
			<< freeBlocksPercentage( POINT ) << " fraction" << endl << endl
			<< "Free Extended Point blocks: " << freeBlocks( EXTENDED_POINT ) << ", "
			<< freeBlocksPercentage( EXTENDED_POINT ) << " fraction" << endl << endl
			<< "Free Node blocks: " << freeBlocks( NODE ) << ", "
			<< freeBlocksPercentage( NODE ) << " fraction" << endl << endl
			<< "=== End of MemManager ===" << endl << endl;
		
		return ss.str();
	}
	
	inline bool MemoryManager::hasEnoughMemory( const float& percentageThreshold ) const
	{
		bool hasMemory = true;
		
		for( int memPoolType = SHALLOW_MORTON; memPoolType < COUNT; ++memPoolType )
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
	
	inline uint MemoryManager::numBlocks( const MANAGED_TYPE_FLAG& type ) const
	{
		return m_pools[ type ].getNumBlocks();
	}
	
	inline uint MemoryManager::freeBlocks( const MANAGED_TYPE_FLAG& type ) const
	{
		return m_pools[ type ].getNumFreeBlocks();
	}
	
	inline float MemoryManager::freeBlocksPercentage( const MANAGED_TYPE_FLAG& type ) const
	{
		return m_pools[ type ].getFreeBlockPercentage();
	}
	
	inline MemoryManager::MemoryManager( const ulong& nShallowMorton, const ulong& nMediumMorton, const ulong& nPoints,
										 const ulong& nExtendedPoints, const ulong& nNodes )
	{
		m_pools[ SHALLOW_MORTON ].createPool( M_SIZES[ SHALLOW_MORTON ], nShallowMorton );
		m_pools[ MEDIUM_MORTON ].createPool( M_SIZES[ MEDIUM_MORTON ], nMediumMorton );
		m_pools[ POINT ].createPool( M_SIZES[ POINT ], nPoints );
		m_pools[ EXTENDED_POINT ].createPool( M_SIZES[ EXTENDED_POINT ], nExtendedPoints );
		m_pools[ NODE ].createPool( M_SIZES[ NODE ], nNodes );
	}
}

#endif