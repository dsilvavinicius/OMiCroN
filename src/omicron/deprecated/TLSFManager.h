#ifndef TLSF_MANAGER_H
#define TLSF_MANAGER_H
#include "MemoryManager.h"
#include "TLSFPool.h"

namespace omicron
{
	template< typename Morton, typename Point, typename Node >
	class TLSFManager
	: public MemoryManager< Morton, Point, Node, ManagedAllocGroup< Morton, Point, Node > >
	{
		using AllocGroup = model::ManagedAllocGroup< Morton, Point, Node >;
		using MemoryManager = model::MemoryManager< Morton, Point, Node, AllocGroup >;
		
		using MortonPtr = shared_ptr< Morton >;
		using MortonPtrInternals = model::PtrInternals< Morton, typename AllocGroup::MortonAlloc >;
		
		using PointPtr = shared_ptr< Point >;
		using PointPtrInternals = model::PtrInternals< Point, typename AllocGroup::PointAlloc >;
		
		using NodePtr = shared_ptr< Node >;
		using NodePtrInternals = model::PtrInternals< Node, typename AllocGroup::NodeAlloc >;
		
		using MapInternals = model::MapInternals< Morton, Node >;
	public:
		static void initInstance( const size_t& maxAllowedMem );
	
	private:
		TLSFManager( const size_t& maxAllowedMem );
	};
	
	template< typename Morton, typename Point, typename Node >
	void TLSFManager< Morton, Point, Node >::initInstance( const size_t& maxAllowedMem )
	{
		MemoryManager::m_instance = unique_ptr< IMemoryManager >(
			new TLSFManager< Morton, Point, Node >( maxAllowedMem )
		);
	}
	
	template< typename Morton, typename Point, typename Node >
	TLSFManager< Morton, Point, Node >::TLSFManager( const size_t& maxAllowedMem )
	{
		MemoryManager::m_MortonPool = new TLSFPool< Morton >();
		MemoryManager::m_MortonPtrPool = new TLSFPool< MortonPtr >();
		MemoryManager::m_MortonPtrInternalsPool = new TLSFPool< MortonPtrInternals >();
		
		MemoryManager::m_IndexPool = new TLSFPool< Index >();
		
		MemoryManager::m_PointPool = new TLSFPool< Point >();
		MemoryManager::m_PointPtrPool = new TLSFPool< PointPtr >();
		MemoryManager::m_PointPtrInternalsPool = new TLSFPool< PointPtrInternals >();
		
		MemoryManager::m_NodePool = new TLSFPool< Node >();
		MemoryManager::m_NodePtrPool = new TLSFPool< NodePtr >();
		MemoryManager::m_NodePtrInternalsPool = new TLSFPool< NodePtrInternals >();
		
		MemoryManager::m_MapInternalsPool = new TLSFPool< MapInternals >();
		
		MemoryManager::m_maxAllowedMem = maxAllowedMem;
	}
}

#endif