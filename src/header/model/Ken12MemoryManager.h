#ifndef KEN_12_MEMORY_MANAGER_H
#define KEN_12_MEMORY_MANAGER_H

#include "MemoryManager.h"
#include "Ken12MemoryPool.h"

namespace model
{
	/** Defines allocator types for Morton, Point and Node types. */
	template< typename Morton, typename Point, typename Node >
	struct DefaultAllocGroup
	{
		using MortonAlloc = std::allocator< Morton >;
		using PointAlloc = std::allocator< Point >;
		using NodeAlloc = std::allocator< Node >;
	};
	
	/** MemoryManager implementation using pools described in Ben Kenwright's Fast Efficient Fixed-Sized Memory Pool paper:
	 * http://www.thinkmind.org/index.php?view=article&articleid=computation_tools_2012_1_10_80006. ARRAY ALLOCATIONS AND
	 * DEALLOCATIONS ARE NOT SUPPORTED BY THIS IMPLEMENTATION.
	 */
	template< typename Morton, typename Point, typename Node >
	class Ken12MemoryManager
	: public MemoryManager< Morton, Point, Node, DefaultAllocGroup< Morton, Point, Node > >
	{
		using AllocGroup = DefaultAllocGroup< Morton, Point, Node >;
		using MemoryManager = model::MemoryManager< Morton, Point, Node, AllocGroup >;
		
		using MortonPtr = shared_ptr< Morton >;
		using MortonPtrInternals = model::PtrInternals< Morton, typename AllocGroup::MortonAlloc >;
		
		using PointPtr = shared_ptr< Point >;
		using PointPtrInternals = model::PtrInternals< Point, typename AllocGroup::PointAlloc >;
		
		using NodePtr = shared_ptr< Node >;
		using NodePtrInternals = model::PtrInternals< Node, typename AllocGroup::NodeAlloc >;
		
		using MapInternals = model::MapInternals< Morton, Node >;
	public:
		/** Initializes the singleton instance with the number of memory blocks for each type specified by the
		 * parameters. If it is already initialized, early allocated memory is deleted and new allocations are done
		 * accordingly with parameters.
		 * @param nMorton is the number of Morton memory blocks.
		 * @param nPoints is the number of Point memory blocks.
		 * @param nNodes is the number of Node memory blocks. */
		static void initInstance( const ulong& nMorton, const ulong& nPoints, const ulong& nNodes );
		
	private:
		/** Initializes instance with the number of memory blocks for each type specified by the parameters.
		 * @param nMorton is the number of Morton memory blocks.
		 * @param nPoints is the number of Point memory blocks.
		 * @param nNodes is the number of Node memory blocks. */
		Ken12MemoryManager( const ulong& nMorton, const ulong& nPoints, const ulong& nNodes );
	};
	
	template< typename Morton, typename Point, typename Node >
	void Ken12MemoryManager< Morton, Point, Node >::initInstance( const ulong& nMorton, const ulong& nPoints,
																  const ulong& nNodes )
	{
		MemoryManager::m_instance = unique_ptr< IMemoryManager >(
			new Ken12MemoryManager< Morton, Point, Node >( nMorton, nPoints, nNodes )
		);
	}
	
	template< typename Morton, typename Point, typename Node >
	Ken12MemoryManager< Morton, Point, Node >::Ken12MemoryManager( const ulong& nMorton, const ulong& nPoints,
																   const ulong& nNodes )
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
		
		auto nodePool = new Ken12MemoryPool< Node >(); nodePool->createPool( nNodes );
		MemoryManager::m_NodePool = nodePool;
		
		auto nodePtrPool = new Ken12MemoryPool< NodePtr >(); nodePtrPool->createPool( 0 );
		MemoryManager::m_NodePtrPool = nodePtrPool;
		
		auto nodePtrInternalsPool = new Ken12MemoryPool< NodePtrInternals >(); nodePtrInternalsPool->createPool( 0 );
		MemoryManager::m_NodePtrInternalsPool = nodePtrInternalsPool;
		
		auto mapInternalsPool = new Ken12MemoryPool< MapInternals >(); mapInternalsPool->createPool( 0 );
		MemoryManager::m_MapInternalsPool = mapInternalsPool;
		
		MemoryManager::m_maxAllowedMem = 	nMorton * sizeof( Morton ) + nPoints * sizeof( Point )
											+ nNodes * sizeof( Node );
	}
}

#endif