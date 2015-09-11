#include <sstream>
#include "BitMapMemoryManager.h"

namespace model
{
	static void BitMapMemoryManager::initInstance( const size_t& maxAllowedMem )
	{
		m_instance = unique_ptr< IMemoryManager >( new BitMapMemoryManager( maxAllowedMem ) );
	}
	
	template< typename T >
	inline void* BitMapMemoryManager::allocate( const size_t& size )
	{
		if( size == sizeof( T ) )
		{
			return getPool< T >().allocate();
		}
		else
		{
			return getPool< T >().allocateArray( size );
		}
	}
	
	template< typename T >
	inline void BitMapMemoryManager::deallocate( void* p )
	{
		getPool< T >().deallocate( p );
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
	
	inline size_t BitMapMemoryManager::usedMemory() const
	{
		return 	m_shallowMortonPool.calcMemoryUsage() + m_mediumMortonPool.calcMemoryUsage()
				+ m_pointPool.calcMemoryUsage() + m_extendedPointPool.calcMemoryUsage() + m_nodePool.calcMemoryUsage();
	}
	
	template<>
	inline BitMemoryPool< ShallowMortonCode >& BitMapMemoryManager::getPool()
	{
		return m_shallowMortonPool;
	}
	
	template<>
	inline BitMemoryPool< MediumMortonCode >& BitMapMemoryManager::getPool()
	{
		return m_mediumMortonPool;
	}
	
	template<>
	inline BitMemoryPool< Point >& BitMapMemoryManager::getPool()
	{
		return m_pointPool;
	}
	
	template<>
	inline BitMemoryPool< ExtendedPoint >& BitMapMemoryManager::getPool()
	{
		return m_extendedPointPool;
	}
	
	template<>
	inline BitMemoryPool< ShallowLeafNode >& BitMapMemoryManager::getPool()
	{
		return m_nodePool;
	}
}