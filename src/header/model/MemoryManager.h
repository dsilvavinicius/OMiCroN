#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include "MemoryPool.h"
#include <vector>

namespace model
{
	/** Utilizes Memory pools in order to manage creation of MortonCode, Point and Node objects. Allocates one large
	 * memory chunk to be served for each type. Reuses deallocated memory for next allocations. */
	class MemoryManager
	{
	public:
		static void initInstance( const ulong& nShallowMorton, const ulong& nMediumMorton, const ulong& nPoints,
								  const ulong& nExtendedPoints, const ulong& nNodes );
		
		/** Gets the MemoryManager singleton instance. */
		static MemoryManager& instance();
		
		/** Allocates memory for a ShallowMortonCode. */
		void* allocateShallowMorton();
		
		/** Allocates memory for a MediumMortonCode. */
		void* allocateMediumMorton();
		
		/** Allocates memory for a Point. */
		void* allocatePoint();
		
		/** Allocates memory for an ExtendedPoint. */
		void* allocateExtendedPoint();
		
		/** Allocates memory for a Node. */
		void* allocateNode();
		
		/** Deallocates memory for a ShallowMortonCode. */
		void deallocateShallowMorton( void* p );
		
		/** Deallocates memory for a MediumMortonCode. */
		void deallocateMediumMorton( void* p );
		
		/** Deallocates memory for a Point. */
		void deallocatePoint( void* p );
		
		/** Deallocates memory for an ExtendedPoint. */
		void deallocateExtendedPoint( void* p );
		
		/** Deallocates memory for a Node. */
		void deallocateNode( void* p );
		
	private:
		/** Ctor doesn't allocates memory. Use initInstance to initialize it. */
		MemoryManager() {}
		
		void init( const ulong& nShallowMorton, const ulong& nMediumMorton, const ulong& nPoints,
				   const ulong& nExtendedPoints, const ulong& nNodes );
		
		MemoryPool m_ShallowMortonPool; // 4 bytes block.
		MemoryPool m_MediumMortonPool; // 8 bytes block.
		MemoryPool m_PointPool; // 24 bytes block.
		MemoryPool m_ExtendedPointPool; // 36 bytes block.
		MemoryPool m_LeafOrInnerNodePool; // 24 bytes block.
		
		static uint SHALLOW_MORTON_SIZE;
		static uint MEDIUM_MORTON_SIZE;
		static uint POINT_SIZE;
		static uint EXTENDED_POINT_SIZE;
		static uint NODE_SIZE;
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

	inline void* MemoryManager::allocateShallowMorton()
	{
		return m_ShallowMortonPool.allocate();
	}
		
	inline void* MemoryManager::allocateMediumMorton()
	{
		return m_MediumMortonPool.allocate();
	}
	
	inline void* MemoryManager::allocatePoint()
	{
		return m_PointPool.allocate();
	}
	
	inline void* MemoryManager::allocateExtendedPoint()
	{
		return m_ExtendedPointPool.allocate();
	}

	inline void* MemoryManager::allocateNode()
	{
		return m_LeafOrInnerNodePool.allocate();
	}
	
	inline void MemoryManager::deallocateShallowMorton( void* p )
	{
		m_ShallowMortonPool.deAllocate( p );
	}
	
	inline void MemoryManager::deallocateMediumMorton( void* p )
	{
		m_MediumMortonPool.deAllocate( p );
	}
	
	inline void MemoryManager::deallocatePoint(void* p)
	{
		m_PointPool.deAllocate( p );
	}
	
	inline void MemoryManager::deallocateExtendedPoint( void* p )
	{
		m_ExtendedPointPool.deAllocate( p );
	}
	
	inline void MemoryManager::deallocateNode( void* p )
	{
		m_LeafOrInnerNodePool.deAllocate( p );
	}
	
	inline void MemoryManager::init( const ulong& nShallowMorton, const ulong& nMediumMorton, const ulong& nPoints,
									 const ulong& nExtendedPoints, const ulong& nNodes )
	{
		if( nShallowMorton )
		{
			m_ShallowMortonPool.createPool( SHALLOW_MORTON_SIZE, nShallowMorton );
		}
		if( nMediumMorton )
		{
			m_MediumMortonPool.createPool( MEDIUM_MORTON_SIZE, nMediumMorton );
		}
		if( nPoints )
		{
			m_PointPool.createPool( POINT_SIZE, nPoints );
		}
		if( nExtendedPoints )
		{
			m_ExtendedPointPool.createPool( EXTENDED_POINT_SIZE, nExtendedPoints );
		}
		if( nNodes )
		{
			m_LeafOrInnerNodePool.createPool( NODE_SIZE, nNodes );
		}
	}
}

#endif