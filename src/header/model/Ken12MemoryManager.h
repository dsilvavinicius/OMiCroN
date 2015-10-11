#ifndef KEN_12_MEMORY_MANAGER_H
#define KEN_12_MEMORY_MANAGER_H

#include "MemoryManager.h"
#include "Ken12MemoryPool.h"

namespace model
{
	/** Defines allocator types for Morton, Point, Inner and Leaf types. */
	template< typename Morton, typename Point, typename Inner, typename Leaf >
	struct DefaultAllocGroup
	{
		using MortonAlloc = std::allocator< Morton >;
		using PointAlloc = std::allocator< Point >;
		using InnerAlloc = std::allocator< Inner >;
		using LeafAlloc = std::allocator< Leaf >;
	};
	
	/** MemoryManager implementation using pools described in Ben Kenwright's Fast Efficient Fixed-Sized Memory Pool paper:
	 * http://www.thinkmind.org/index.php?view=article&articleid=computation_tools_2012_1_10_80006. ARRAY ALLOCATIONS AND
	 * DEALLOCATIONS ARE NOT SUPPORTED BY THIS IMPLEMENTATION.
	 */
	template< typename Morton, typename Point, typename Inner, typename Leaf >
	class Ken12MemoryManager
	: public MemoryManager< Morton, Point, Inner, Leaf, DefaultAllocGroup< Morton, Point, Inner, Leaf > >
	{
		using AllocGroup = DefaultAllocGroup< Morton, Point, Inner, Leaf >;
		using MemoryManager = model::MemoryManager< Morton, Point, Inner, Leaf, AllocGroup >;
		
		using MortonPtr = shared_ptr< Morton >;
		using MortonPtrInternals = model::PtrInternals< Morton, typename AllocGroup::MortonAlloc >;
		
		using PointPtr = shared_ptr< Point >;
		using PointPtrInternals = model::PtrInternals< Point, typename AllocGroup::PointAlloc >;
		
		using InnerPtr = shared_ptr< Inner >;
		using InnerPtrInternals = model::PtrInternals< Inner, typename AllocGroup::InnerAlloc >;
		
		using LeafPtr = shared_ptr< Leaf >;
		using LeafPtrInternals = model::PtrInternals< Leaf, typename AllocGroup::LeafAlloc >;
		
		using MapInternals = model::MapInternals< Morton >;
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
		MemoryManager::m_MortonPool = mortonPool;
		
		auto mortonPtrPool = new Ken12MemoryPool< MortonPtr >(); mortonPtrPool->createPool( 0 );
		MemoryManager::m_MortonPtrPool = mortonPtrPool;
		
		auto mortonPtrInternalsPool = new Ken12MemoryPool< MortonPtrInternals >(); mortonPtrInternalsPool->createPool( 0 );
		MemoryManager::m_MortonPtrInternalsPool = mortonPtrInternalsPool;
		
		auto indexPool = new Ken12MemoryPool< Index >(); indexPool->createPool( 0 );
		MemoryManager::m_IndexPool = indexPool;
		
		auto pointPool = new Ken12MemoryPool< Point >(); pointPool->createPool( nPoints );
		MemoryManager::m_PointPool = pointPool;
		
		auto pointPtrPool = new Ken12MemoryPool< PointPtr >(); pointPtrPool->createPool( 0 );
		MemoryManager::m_PointPtrPool = pointPtrPool;
		
		auto pointPtrInternalsPool = new Ken12MemoryPool< PointPtrInternals >(); pointPtrInternalsPool->createPool( 0 );
		MemoryManager::m_PointPtrInternalsPool = pointPtrInternalsPool;
		
		auto innerPool = new Ken12MemoryPool< Inner >(); innerPool->createPool( nInners );
		MemoryManager::m_InnerPool = innerPool;
		
		auto innerPtrPool = new Ken12MemoryPool< InnerPtr >(); innerPtrPool->createPool( 0 );
		MemoryManager::m_InnerPtrPool = innerPtrPool;
		
		auto innerPtrInternalsPool = new Ken12MemoryPool< InnerPtrInternals >(); innerPtrInternalsPool->createPool( 0 );
		MemoryManager::m_InnerPtrInternalsPool = innerPtrInternalsPool;
		
		auto leafPool = new Ken12MemoryPool< Leaf >(); leafPool->createPool( nLeaves );
		MemoryManager::m_LeafPool = leafPool;
		
		auto leafPtrPool = new Ken12MemoryPool< LeafPtr >(); leafPtrPool->createPool( 0 );
		MemoryManager::m_LeafPtrPool = leafPtrPool;
		
		auto leafPtrInternalsPool = new Ken12MemoryPool< LeafPtrInternals >(); leafPtrInternalsPool->createPool( 0 );
		MemoryManager::m_LeafPtrInternalsPool = leafPtrInternalsPool;
		
		auto mapInternalsPool = new Ken12MemoryPool< MapInternals >(); mapInternalsPool->createPool( 0 );
		MemoryManager::m_MapInternalsPool = mapInternalsPool;
		
		MemoryManager::m_maxAllowedMem = 	nMorton * sizeof( Morton ) + nPoints * sizeof( Point )
											+ nInners * sizeof( Inner ) + nLeaves * sizeof( Leaf );
	}
}

#endif