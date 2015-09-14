#include <sstream>
#include "BitMapMemoryManager.h"

namespace model
{
	void BitMapMemoryManager::initInstance( const size_t& maxAllowedMem )
	{
		m_instance = unique_ptr< IMemoryManager >( new BitMapMemoryManager( maxAllowedMem ) );
	}
	
	inline void* BitMapMemoryManager::allocate( const MANAGED_TYPE_FLAG& type )
	{
		switch( type )
		{
			case SHALLOW_MORTON		: return m_shallowMortonPool.allocate();
			case MEDIUM_MORTON		: return m_mediumMortonPool.allocate();
			case POINT				: return m_pointPool.allocate();
			case EXTENDED_POINT		: return m_extendedPointPool.allocate();
			case NODE				: return m_nodePool.allocate();
			default					: throw logic_error( "Unknowm nanaged type" );
		}
	}
	
	inline void* BitMapMemoryManager::allocateArray( const size_t& size, const MANAGED_TYPE_FLAG& type )
	{
		switch( type )
		{
			case SHALLOW_MORTON		: return m_shallowMortonPool.allocateArray( size );
			case MEDIUM_MORTON		: return m_mediumMortonPool.allocateArray( size );
			case POINT				: return m_pointPool.allocateArray( size );
			case EXTENDED_POINT		: return m_extendedPointPool.allocateArray( size );
			case NODE				: return m_nodePool.allocateArray( size );
			default					: throw logic_error( "Unknowm nanaged type" );
		}
	}
	
	inline void BitMapMemoryManager::deallocate( void* p, const MANAGED_TYPE_FLAG& type )
	{
		switch( type )
		{
			case SHALLOW_MORTON		: return m_shallowMortonPool.deallocate( p );
			case MEDIUM_MORTON		: return m_mediumMortonPool.deallocate( p );
			case POINT				: return m_pointPool.deallocate( p );
			case EXTENDED_POINT		: return m_extendedPointPool.deallocate( p );
			case NODE				: return m_nodePool.deallocate( p );
			default					: throw logic_error( "Unknowm nanaged type" );
		}
	}
	
	inline void BitMapMemoryManager::deallocateArray( void* p, const MANAGED_TYPE_FLAG& type )
	{
		switch( type )
		{
			case SHALLOW_MORTON		: return m_shallowMortonPool.deallocateArray( p );
			case MEDIUM_MORTON		: return m_mediumMortonPool.deallocateArray( p );
			case POINT				: return m_pointPool.deallocateArray( p );
			case EXTENDED_POINT		: return m_extendedPointPool.deallocateArray( p );
			case NODE				: return m_nodePool.deallocateArray( p );
			default					: throw logic_error( "Unknowm nanaged type" );
		}
	}
	
	inline bool BitMapMemoryManager::hasEnoughMemory( const float& percentageThreshold ) const
	{
		return ( m_maxAllowedMem - usedMemory() ) > float( m_maxAllowedMem * percentageThreshold );
	}
	
	string BitMapMemoryManager::toString() const
	{
		cout << "ShallowMorton used blocks: " << m_shallowMortonPool.usedBlocks() << " Used memory: "
			 << m_shallowMortonPool.usedBlocks() << endl
			 << "MediumMorton used blocks: " << m_mediumMortonPool.usedBlocks() << " Used memory: "
			 << m_mediumMortonPool.usedBlocks() << endl
			 << "Point used blocks: " << m_pointPool.usedBlocks() << " Used memory: "
			 << m_pointPool.usedBlocks() << endl
			 << "ExtendedPoint used blocks: " << m_extendedPointPool.usedBlocks() << " Used memory: "
			 << m_extendedPointPool.usedBlocks() << endl
			 << "Node used blocks: " << m_nodePool.usedBlocks() << " Used memory: "
			 << m_nodePool.usedBlocks() << endl << endl;
	}
	
	template<>
	inline BitMapMemoryPool< ShallowMortonCode >& BitMapMemoryManager::getPool()
	{
		return m_shallowMortonPool;
	}
	
	template<>
	inline BitMapMemoryPool< MediumMortonCode >& BitMapMemoryManager::getPool()
	{
		return m_mediumMortonPool;
	}
	
	template<>
	inline BitMapMemoryPool< Point >& BitMapMemoryManager::getPool()
	{
		return m_pointPool;
	}
	
	template<>
	inline BitMapMemoryPool< ExtendedPoint >& BitMapMemoryManager::getPool()
	{
		return m_extendedPointPool;
	}
	
	template<>
	inline BitMapMemoryPool< ShallowLeafNode< PointVector > >& BitMapMemoryManager::getPool()
	{
		return m_nodePool;
	}
}