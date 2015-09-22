#ifndef KEN_12_MEMORY_MANAGER_H
#define KEN_12_MEMORY_MANAGER_H

#include "MemoryManager.h"
#include "Ken12MemoryPool.h"

namespace model
{
	/** MemoryManager implementation using pools described in Ben Kenwright's Fast Efficient Fixed-Sized Memory Pool paper:
	 * http://www.thinkmind.org/index.php?view=article&articleid=computation_tools_2012_1_10_80006. ARRAY ALLOCATIONS AND
	 * DEALLOCATIONS ARE NOT SUPPORTED BY THIS IMPLEMENTATION.
	 */
	template< typename Morton, typename Point, typename Inner, typename Leaf >
	class Ken12MemoryManager
	: public MemoryManager< Morton, Point, Inner, Leaf >
	{
		using MemoryManager = model::MemoryManager< Morton, Point, Inner, Leaf >;
	public:
		/** Initializes the singleton instance with the number of memory blocks for each type specified by the
		 * parameters. If it is already initialized, early allocated memory is deleted and new allocations are done
		 * accordingly with parameters.
		 * @param nMorton is the number of Morton memory blocks.
		 * @param nPoints is the number of Point memory blocks.
		 * @param nInners is the number of Inner memory blocks.
		 * @param nLeaves is the number of Leaf memory blocks.*/
		static void initInstance( const ulong& nMorton, const ulong& nPoints, const ulong& nInners, const ulong& nLeaves );
		
	private:
		/** Initializes instance with the number of memory blocks for each type specified by the parameters.
		 * @param nMorton is the number of Morton memory blocks.
		 * @param nPoints is the number of Point memory blocks.
		 * @param nInners is the number of Inner memory blocks.
		 * @param nLeaves is the number of Leaf memory blocks.*/
		Ken12MemoryManager( const ulong& nMorton, const ulong& nPoints, const ulong& nInners, const ulong& nLeaves );
	};
	
	template< typename Morton, typename Point, typename Inner, typename Leaf >
	void Ken12MemoryManager< Morton, Point, Inner, Leaf >::initInstance( const ulong& nMorton, const ulong& nPoints,
																		 const ulong& nInners, const ulong& nLeaves )
	{
		MemoryManager::m_instance = unique_ptr< IMemoryManager >(
			new Ken12MemoryManager< Morton, Point, Inner, Leaf >( nMorton, nPoints, nInners, nLeaves )
		);
	}
	
	template< typename Morton, typename Point, typename Inner, typename Leaf >
	Ken12MemoryManager< Morton, Point, Inner, Leaf >::Ken12MemoryManager( const ulong& nMorton, const ulong& nPoints,
																		  const ulong& nInners, const ulong& nLeaves )
	{
		auto mortonPool = new Ken12MemoryPool< Morton >(); mortonPool->createPool( nMorton );
		MemoryManager::m_mortonPool = mortonPool;
		
		auto pointPool = new Ken12MemoryPool< Point >(); pointPool->createPool( nPoints );
		MemoryManager::m_pointPool = pointPool;
		
		auto innerPool = new Ken12MemoryPool< Inner >(); innerPool->createPool( nInners );
		MemoryManager::m_innerPool = innerPool;
		
		auto leafPool = new Ken12MemoryPool< Leaf >(); leafPool->createPool( nInners );
		MemoryManager::m_leafPool = leafPool;
		
		MemoryManager::m_maxAllowedMem = 	nMorton * sizeof( Morton ) + nPoints * sizeof( Point )
											+ nInners * sizeof( Inner ) + nLeaves * sizeof( Leaf );
	}
}

#endif