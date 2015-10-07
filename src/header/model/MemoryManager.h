#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <vector>
#include <sstream>
#include "IMemoryPool.h"
#include "IMemoryManager.h"
#include "BitMapAllocator.h"

namespace model
{
	// Macro that declares everything in MemoryManager related with a given memory pool type.
	#define DECLARE_POOL_MEMBERS(TYPE) \
	void* alloc##TYPE() override; \
	\
	void* alloc##TYPE##Array( const size_t& size ) override;\
	\
	void dealloc##TYPE( void* p ) override;\
	\
	void dealloc##TYPE##Array( void* p ) override;\
	\
	size_t num##TYPE##Blocks() const;
	
	// Macro that defines everything in MemoryManager related with a given memory pool type.
	#define DEFINE_POOL_MEMBERS(TYPE) \
	template< typename Morton, typename Point, typename Inner, typename Leaf, typename AllocGroup >\
	inline void* MemoryManager< Morton, Point, Inner, Leaf, AllocGroup >::alloc##TYPE()\
	{\
		return m_##TYPE##Pool->allocate();\
	}\
	\
	template< typename Morton, typename Point, typename Inner, typename Leaf, typename AllocGroup >\
	void* MemoryManager< Morton, Point, Inner, Leaf, AllocGroup >::alloc##TYPE##Array( const size_t& size )\
	{\
		return m_##TYPE##Pool->allocateArray( size );\
	}\
	\
	template< typename Morton, typename Point, typename Inner, typename Leaf, typename AllocGroup >\
	inline void MemoryManager< Morton, Point, Inner, Leaf, AllocGroup >::dealloc##TYPE( void* p )\
	{\
		m_##TYPE##Pool->deallocate( static_cast< TYPE* >( p ) );\
	}\
	\
	template< typename Morton, typename Point, typename Inner, typename Leaf, typename AllocGroup >\
	void MemoryManager< Morton, Point, Inner, Leaf, AllocGroup >::dealloc##TYPE##Array( void* p )\
	{\
		m_##TYPE##Pool->deallocateArray( static_cast< TYPE* >( p ) );\
	}\
	
	/** Skeletal base implementation of a SingletonMemoryManager. This implementation does not initialize members, what
	 * is left for derived classes to do.
	 * @param Morton is the morton code type.
	 * @param Point is the point Type.
	 * @param Inner is the inner node type.
	 * @param Leaf is the leaf node type.
	 * @param AllocGroup is a struct defining the allocators for Morton, Point, Inner and Leaf types.
	 */
	template< typename Morton, typename Point, typename Inner, typename Leaf, typename AllocGroup >
	class MemoryManager
	: public SingletonMemoryManager
	{
		using PointPtr = shared_ptr< Point >;
		using MortonPtr = shared_ptr< Morton >;
		using InnerPtr = shared_ptr< Inner >;
		using LeafPtr = shared_ptr< Leaf >;
		
		using PointPtrInternals = model::PtrInternals< Point, typename AllocGroup::PointAlloc >;
		using MortonPtrInternals = model::PtrInternals< Morton, typename AllocGroup::MortonAlloc >;
		using InnerPtrInternals = model::PtrInternals< Inner, typename AllocGroup::InnerAlloc >;
		using LeafPtrInternals = model::PtrInternals< Leaf, typename AllocGroup::LeafAlloc >;
		
		using MapInternals = model::MapInternals< Morton >;
	public:
		virtual ~MemoryManager() = 0;
		
		template< typename T >
		struct TypeWrapper{};
		
		DECLARE_POOL_MEMBERS(Morton)
		DECLARE_POOL_MEMBERS(MortonPtr)
		DECLARE_POOL_MEMBERS(MortonPtrInternals)
		
		DECLARE_POOL_MEMBERS(Index)
		
		DECLARE_POOL_MEMBERS(Point)
		DECLARE_POOL_MEMBERS(PointPtr)
		DECLARE_POOL_MEMBERS(PointPtrInternals)
		
		DECLARE_POOL_MEMBERS(Inner)
		DECLARE_POOL_MEMBERS(InnerPtr)
		DECLARE_POOL_MEMBERS(InnerPtrInternals)
		
		DECLARE_POOL_MEMBERS(Leaf)
		DECLARE_POOL_MEMBERS(LeafPtr)
		DECLARE_POOL_MEMBERS(LeafPtrInternals)
		
		DECLARE_POOL_MEMBERS(MapInternals)
		
		size_t usedMemory() const override;
		
		size_t maxAllowedMem() const override;
		
		bool hasEnoughMemory( const float& percentageThreshold ) const override;
		
		string toString() const override;
		
	protected:
		IMemoryPool< Morton >* m_MortonPool;
		IMemoryPool< MortonPtr >* m_MortonPtrPool;
		IMemoryPool< MortonPtrInternals >* m_MortonPtrInternalsPool;
		
		IMemoryPool< Index >* m_IndexPool;
		
		IMemoryPool< Point >* m_PointPool;
		IMemoryPool< PointPtr >* m_PointPtrPool;
		IMemoryPool< PointPtrInternals >* m_PointPtrInternalsPool;
		
		IMemoryPool< Inner >* m_InnerPool;
		IMemoryPool< InnerPtr >* m_InnerPtrPool;
		IMemoryPool< InnerPtrInternals >* m_InnerPtrInternalsPool;
		
		IMemoryPool< Leaf >* m_LeafPool;
		IMemoryPool< LeafPtr >* m_LeafPtrPool;
		IMemoryPool< LeafPtrInternals >* m_LeafPtrInternalsPool;
		
		IMemoryPool< MapInternals >* m_MapInternalsPool;
		size_t m_maxAllowedMem;
	};
	
	template< typename Morton, typename Point, typename Inner, typename Leaf, typename AllocGroup >
	MemoryManager< Morton, Point, Inner, Leaf, AllocGroup >::~MemoryManager()
	{
		delete m_MortonPool;
		delete m_MortonPtrPool;
		delete m_MortonPtrInternalsPool;
		
		delete m_IndexPool;
		
		delete m_PointPool;
		delete m_PointPtrPool;
		delete m_PointPtrInternalsPool;
		
		delete m_InnerPool;
		delete m_InnerPtrPool;
		delete m_InnerPtrInternalsPool;
		
		delete m_LeafPool;
		delete m_LeafPtrPool;
		delete m_LeafPtrInternalsPool;
		
		delete m_MapInternalsPool;
	}
	
	DEFINE_POOL_MEMBERS(Morton)
	DEFINE_POOL_MEMBERS(MortonPtr)
	DEFINE_POOL_MEMBERS(MortonPtrInternals)
	
	DEFINE_POOL_MEMBERS(Index)
	
	DEFINE_POOL_MEMBERS(Point)
	DEFINE_POOL_MEMBERS(PointPtr)
	DEFINE_POOL_MEMBERS(PointPtrInternals)
	
	DEFINE_POOL_MEMBERS(Inner)
	DEFINE_POOL_MEMBERS(InnerPtr)
	DEFINE_POOL_MEMBERS(InnerPtrInternals)
	
	DEFINE_POOL_MEMBERS(Leaf)
	DEFINE_POOL_MEMBERS(LeafPtr)
	DEFINE_POOL_MEMBERS(LeafPtrInternals)
	
	DEFINE_POOL_MEMBERS(MapInternals)
	
	template< typename Morton, typename Point, typename Inner, typename Leaf, typename AllocGroup >
	inline size_t MemoryManager< Morton, Point, Inner, Leaf, AllocGroup >::usedMemory() const
	{
		return 	m_MortonPool->memoryUsage() + m_MortonPtrPool->memoryUsage() + m_MortonPtrInternalsPool->memoryUsage() +
				m_IndexPool->memoryUsage() + m_PointPool->memoryUsage() + m_PointPtrPool->memoryUsage() +
				m_PointPtrInternalsPool->memoryUsage() + m_InnerPool->memoryUsage() + m_InnerPtrPool->memoryUsage() +
				m_InnerPtrInternalsPool->memoryUsage() + m_LeafPool->memoryUsage() + m_LeafPtrPool->memoryUsage() +
				m_LeafPtrInternalsPool->memoryUsage() + m_MapInternalsPool->memoryUsage();
	}
	
	template< typename Morton, typename Point, typename Inner, typename Leaf, typename AllocGroup >
	size_t MemoryManager< Morton, Point, Inner, Leaf, AllocGroup >::maxAllowedMem() const
	{
		return m_maxAllowedMem;
	}
	
	template< typename Morton, typename Point, typename Inner, typename Leaf, typename AllocGroup >
	inline bool MemoryManager< Morton, Point, Inner, Leaf, AllocGroup >::hasEnoughMemory( const float& percentageThreshold ) const
	{
		return ( m_maxAllowedMem - usedMemory() ) > float( m_maxAllowedMem * percentageThreshold );
	}
	
	template< typename Morton, typename Point, typename Inner, typename Leaf, typename AllocGroup >
	string MemoryManager< Morton, Point, Inner, Leaf, AllocGroup >::toString() const
	{
		stringstream ss;
		ss 	<< "Morton used blocks: " << m_MortonPool->usedBlocks() << " Used memory: " << m_MortonPool->memoryUsage() << endl
			<< "MortonPtr used blocks: " << m_MortonPtrPool->usedBlocks() << " Used memory: " << m_MortonPtrPool->memoryUsage() << endl
			<< "MortonPtrInternals used blocks: " << m_MortonPtrInternalsPool->usedBlocks() << " Used memory: " << m_MortonPtrInternalsPool->memoryUsage() << endl
			
			<< "Index used blocks: " << m_IndexPool->usedBlocks() << " Used memory: " << m_IndexPool->memoryUsage() << endl
			
			<< "Point used blocks: " << m_PointPool->usedBlocks() << " Used memory: " << m_PointPool->memoryUsage() << endl
			<< "PointPtr used blocks: " << m_PointPtrPool->usedBlocks() << " Used memory: " << m_PointPtrPool->memoryUsage() << endl
			<< "PointPtrInternals used blocks: " << m_PointPtrInternalsPool->usedBlocks() << " Used memory: " << m_PointPtrInternalsPool->memoryUsage() << endl
			
			<< "Inner used blocks: " << m_InnerPool->usedBlocks() << " Used memory: " << m_InnerPool->memoryUsage() << endl
			<< "InnerPtr used blocks: " << m_InnerPtrPool->usedBlocks() << " Used memory: " << m_InnerPtrPool->memoryUsage() << endl
			<< "InnerPtrInternals used blocks: " << m_InnerPtrInternalsPool->usedBlocks() << " Used memory: " << m_InnerPtrInternalsPool->memoryUsage() << endl
			
			<< "Leaf used blocks: " << m_LeafPool->usedBlocks() << " Used memory: " << m_LeafPool->memoryUsage() << endl << endl
			<< "LeafPtr used blocks: " << m_LeafPtrPool->usedBlocks() << " Used memory: " << m_LeafPtrPool->memoryUsage() << endl
			<< "LeafPtrInternals used blocks: " << m_LeafPtrInternalsPool->usedBlocks() << " Used memory: " << m_LeafPtrInternalsPool->memoryUsage() << endl
			
			<< "MapInternals used blocks: " << m_MapInternalsPool->usedBlocks() << " Used memory: " << m_MapInternalsPool->memoryUsage() << endl;
		return ss.str();
	}
}

#endif