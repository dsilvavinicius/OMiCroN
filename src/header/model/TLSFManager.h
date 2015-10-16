#ifndef TLSF_MANAGER_H
#define TLSF_MANAGER_H
#include "MemoryManager.h"
#include "TLSFPool.h"

namespace model
{
	template< typename Morton, typename Point, typename Inner, typename Leaf >
	class TLSFManager
	: public MemoryManager< Morton, Point, Inner, Leaf, ManagedAllocGroup< Morton, Point, Inner, Leaf > >
	{
		using AllocGroup = model::ManagedAllocGroup< Morton, Point, Inner, Leaf >;
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
		static void initInstance( const size_t& maxAllowedMem );
	
	private:
		TLSFManager( const size_t& maxAllowedMem );
	};
	
	template< typename Morton, typename Point, typename Inner, typename Leaf >
	void TLSFManager< Morton, Point, Inner, Leaf >::initInstance( const size_t& maxAllowedMem )
	{
		MemoryManager::m_instance = unique_ptr< IMemoryManager >(
			new TLSFManager< Morton, Point, Inner, Leaf >( maxAllowedMem )
		);
	}
	
	template< typename Morton, typename Point, typename Inner, typename Leaf >
	TLSFManager< Morton, Point, Inner, Leaf >::TLSFManager( const size_t& maxAllowedMem )
	{
		MemoryManager::m_MortonPool = new TLSFPool< Morton >();
		MemoryManager::m_MortonPtrPool = new TLSFPool< MortonPtr >();
		MemoryManager::m_MortonPtrInternalsPool = new TLSFPool< MortonPtrInternals >();
		
		MemoryManager::m_IndexPool = new TLSFPool< Index >();
		
		MemoryManager::m_PointPool = new TLSFPool< Point >();
		MemoryManager::m_PointPtrPool = new TLSFPool< PointPtr >();
		MemoryManager::m_PointPtrInternalsPool = new TLSFPool< PointPtrInternals >();
		
		MemoryManager::m_InnerPool = new TLSFPool< Inner >();
		MemoryManager::m_InnerPtrPool = new TLSFPool< InnerPtr >();
		MemoryManager::m_InnerPtrInternalsPool = new TLSFPool< InnerPtrInternals >();
		
		MemoryManager::m_LeafPool = new TLSFPool< Leaf >();
		MemoryManager::m_LeafPtrPool = new TLSFPool< LeafPtr >();
		MemoryManager::m_LeafPtrInternalsPool = new TLSFPool< LeafPtrInternals >();
		
		MemoryManager::m_MapInternalsPool = new TLSFPool< MapInternals >();
		
		MemoryManager::m_maxAllowedMem = maxAllowedMem;
	}
}

#endif