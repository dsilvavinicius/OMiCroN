#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <vector>
#include <sstream>
#include "IMemoryPool.h"
#include "IMemoryManager.h"
#include "ManagedAllocator.h"

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
	template< typename Morton, typename Point, typename Node, typename AllocGroup >\
	inline void* MemoryManager< Morton, Point, Node, AllocGroup >::alloc##TYPE()\
	{\
		return m_##TYPE##Pool->allocate();\
	}\
	\
	template< typename Morton, typename Point, typename Node, typename AllocGroup >\
	inline void* MemoryManager< Morton, Point, Node, AllocGroup >::alloc##TYPE##Array( const size_t& size )\
	{\
		return m_##TYPE##Pool->allocateArray( size );\
	}\
	\
	template< typename Morton, typename Point, typename Node, typename AllocGroup >\
	inline void MemoryManager< Morton, Point, Node, AllocGroup >::dealloc##TYPE( void* p )\
	{\
		m_##TYPE##Pool->deallocate( static_cast< TYPE* >( p ) );\
	}\
	\
	template< typename Morton, typename Point, typename Node, typename AllocGroup >\
	inline void MemoryManager< Morton, Point, Node, AllocGroup >::dealloc##TYPE##Array( void* p )\
	{\
		m_##TYPE##Pool->deallocateArray( static_cast< TYPE* >( p ) );\
	}\
	
	/** Skeletal base implementation of a SingletonMemoryManager. This implementation does not initialize members, what
	 * is left for derived classes to do.
	 * @param Morton is the morton code type.
	 * @param Point is the point Type.
	 * @param Node is the node type.
	 * @param AllocGroup is a struct defining the allocators for Morton, Point and Node types.
	 */
	template< typename Morton, typename Point, typename Node, typename AllocGroup >
	class MemoryManager
	: public SingletonMemoryManager
	{
		using PointPtr = shared_ptr< Point >;
		using MortonPtr = shared_ptr< Morton >;
		using NodePtr = shared_ptr< Node >;
		
		using PointPtrInternals = model::PtrInternals< Point, typename AllocGroup::PointAlloc >;
		using MortonPtrInternals = model::PtrInternals< Morton, typename AllocGroup::MortonAlloc >;
		using NodePtrInternals = model::PtrInternals< Node, typename AllocGroup::NodeAlloc >;
		
		using MapInternals = model::MapInternals< Morton, Node >;
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
		
		DECLARE_POOL_MEMBERS(Node)
		DECLARE_POOL_MEMBERS(NodePtr)
		DECLARE_POOL_MEMBERS(NodePtrInternals)
		
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
		
		IMemoryPool< Node >* m_NodePool;
		IMemoryPool< NodePtr >* m_NodePtrPool;
		IMemoryPool< NodePtrInternals >* m_NodePtrInternalsPool;
		
		IMemoryPool< MapInternals >* m_MapInternalsPool;
		size_t m_maxAllowedMem;
	};
	
	template< typename Morton, typename Point, typename Node, typename AllocGroup >
	MemoryManager< Morton, Point, Node, AllocGroup >::~MemoryManager()
	{
		delete m_MortonPool;
		delete m_MortonPtrPool;
		delete m_MortonPtrInternalsPool;
		
		delete m_IndexPool;
		
		delete m_PointPool;
		delete m_PointPtrPool;
		delete m_PointPtrInternalsPool;
		
		delete m_NodePool;
		delete m_NodePtrPool;
		delete m_NodePtrInternalsPool;
		
		delete m_MapInternalsPool;
	}
	
	DEFINE_POOL_MEMBERS(Morton)
	DEFINE_POOL_MEMBERS(MortonPtr)
	DEFINE_POOL_MEMBERS(MortonPtrInternals)
	
	DEFINE_POOL_MEMBERS(Index)
	
	DEFINE_POOL_MEMBERS(Point)
	DEFINE_POOL_MEMBERS(PointPtr)
	DEFINE_POOL_MEMBERS(PointPtrInternals)
	
	DEFINE_POOL_MEMBERS(Node)
	DEFINE_POOL_MEMBERS(NodePtr)
	DEFINE_POOL_MEMBERS(NodePtrInternals)
	
	DEFINE_POOL_MEMBERS(MapInternals)
	
	template< typename Morton, typename Point, typename Node, typename AllocGroup >
	inline size_t MemoryManager< Morton, Point, Node, AllocGroup >::usedMemory() const
	{
		return 	m_MortonPool->memoryUsage() + m_MortonPtrPool->memoryUsage() + m_MortonPtrInternalsPool->memoryUsage() +
				m_IndexPool->memoryUsage() + m_PointPool->memoryUsage() + m_PointPtrPool->memoryUsage() +
				m_PointPtrInternalsPool->memoryUsage() + m_NodePool->memoryUsage() + m_NodePtrPool->memoryUsage() +
				m_NodePtrInternalsPool->memoryUsage() + m_MapInternalsPool->memoryUsage();
	}
	
	template< typename Morton, typename Point, typename Node, typename AllocGroup >
	inline size_t MemoryManager< Morton, Point, Node, AllocGroup >::maxAllowedMem() const
	{
		return m_maxAllowedMem;
	}
	
	template< typename Morton, typename Point, typename Node, typename AllocGroup >
	inline bool MemoryManager< Morton, Point, Node, AllocGroup >
	::hasEnoughMemory( const float& percentageThreshold ) const
	{
		return ( m_maxAllowedMem - usedMemory() ) > float( m_maxAllowedMem * percentageThreshold );
	}
	
	template< typename Morton, typename Point, typename Node, typename AllocGroup >
	string MemoryManager< Morton, Point, Node, AllocGroup >::toString() const
	{
		stringstream ss;
		ss 	<< "Max allowed mem: " << m_maxAllowedMem << endl << endl
			<< "Used memory: " << usedMemory() << endl << endl
			<< "Morton used blocks: " << m_MortonPool->usedBlocks() << " Used memory: " << m_MortonPool->memoryUsage() << endl << endl
			<< "MortonPtr used blocks: " << m_MortonPtrPool->usedBlocks() << " Used memory: " << m_MortonPtrPool->memoryUsage() << endl << endl
			<< "MortonPtrInternals used blocks: " << m_MortonPtrInternalsPool->usedBlocks() << " Used memory: " << m_MortonPtrInternalsPool->memoryUsage() << endl << endl
			
			<< "Index used blocks: " << m_IndexPool->usedBlocks() << " Used memory: " << m_IndexPool->memoryUsage() << endl << endl
			
			<< "Point used blocks: " << m_PointPool->usedBlocks() << " Used memory: " << m_PointPool->memoryUsage() << endl << endl
			<< "PointPtr used blocks: " << m_PointPtrPool->usedBlocks() << " Used memory: " << m_PointPtrPool->memoryUsage() << endl << endl
			<< "PointPtrInternals used blocks: " << m_PointPtrInternalsPool->usedBlocks() << " Used memory: " << m_PointPtrInternalsPool->memoryUsage() << endl << endl
			
			<< "Node used blocks: " << m_NodePool->usedBlocks() << " Used memory: " << m_NodePool->memoryUsage() << endl << endl
			<< "NodePtr used blocks: " << m_NodePtrPool->usedBlocks() << " Used memory: " << m_NodePtrPool->memoryUsage() << endl << endl
			<< "NodePtrInternals used blocks: " << m_NodePtrInternalsPool->usedBlocks() << " Used memory: " << m_NodePtrInternalsPool->memoryUsage() << endl << endl
			
			<< "MapInternals used blocks: " << m_MapInternalsPool->usedBlocks() << " Used memory: " << m_MapInternalsPool->memoryUsage() << endl << endl;
		return ss.str();
	}
}

#endif