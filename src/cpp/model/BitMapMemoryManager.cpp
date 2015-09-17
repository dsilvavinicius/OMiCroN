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
		//cout << "Allocating " << type << endl << endl;
		switch( type )
		{
			case SHALLOW_MORTON		: return m_shallowMortonPool.allocate();
			case MEDIUM_MORTON		: return m_mediumMortonPool.allocate();
			case POINT				: return m_pointPool.allocate();
			case EXTENDED_POINT		: return m_extendedPointPool.allocate();
			case NODE				: return m_nodePool.allocate();
			default					: throw logic_error( "Unknowm managed type" );
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
			default					: throw logic_error( "Unknowm managed type" );
		}
	}
	
	inline void BitMapMemoryManager::deallocate( void* p, const MANAGED_TYPE_FLAG& type )
	{
		//cout << "Dealloc" << type << endl << endl;
		
		switch( type )
		{
			case SHALLOW_MORTON		: return m_shallowMortonPool.deallocate( static_cast< ShallowMortonCode* >( p ) );
			case MEDIUM_MORTON		: return m_mediumMortonPool.deallocate( static_cast< MediumMortonCode* >( p ) );
			case POINT				: return m_pointPool.deallocate( static_cast< Point* >( p ) );
			case EXTENDED_POINT		: return m_extendedPointPool.deallocate( static_cast< ExtendedPoint* >( p ) );
			case NODE				: return m_nodePool.deallocate( static_cast< ShallowLeafNode< PointVector >* >( p ) );
			default					: throw logic_error( "Unknowm nanaged type" );
		}
	}
	
	inline void BitMapMemoryManager::deallocateArray( void* p, const MANAGED_TYPE_FLAG& type )
	{
		//cout << "Dealloc array " << type << endl << endl;
		
		switch( type )
		{
			case SHALLOW_MORTON		: return m_shallowMortonPool.deallocateArray( static_cast< ShallowMortonCode* >( p ) );
			case MEDIUM_MORTON		: return m_mediumMortonPool.deallocateArray( static_cast< MediumMortonCode* >( p ) );
			case POINT				: return m_pointPool.deallocateArray( static_cast< Point* >( p ) );
			case EXTENDED_POINT		: return m_extendedPointPool.deallocateArray( static_cast< ExtendedPoint* >( p ) );
			case NODE				: return m_nodePool.deallocateArray( static_cast< ShallowLeafNode< PointVector >* >( p ) );
			default					: throw logic_error( "Unknowm nanaged type" );
		}
	}
	
	inline bool BitMapMemoryManager::hasEnoughMemory( const float& percentageThreshold ) const
	{
		return ( m_maxAllowedMem - usedMemory() ) > float( m_maxAllowedMem * percentageThreshold );
	}
	
	string BitMapMemoryManager::toString() const
	{
		stringstream ss;
		ss 	<< "ShallowMorton used blocks: " << m_shallowMortonPool.usedBlocks() << " Used memory: "
			<< m_shallowMortonPool.memoryUsage() << endl
			<< "MediumMorton used blocks: " << m_mediumMortonPool.usedBlocks() << " Used memory: "
			<< m_mediumMortonPool.memoryUsage() << endl
			<< "Point used blocks: " << m_pointPool.usedBlocks() << " Used memory: "
			<< m_pointPool.memoryUsage() << endl
			<< "ExtendedPoint used blocks: " << m_extendedPointPool.usedBlocks() << " Used memory: "
			<< m_extendedPointPool.memoryUsage() << endl
			<< "Node used blocks: " << m_nodePool.usedBlocks() << " Used memory: "
			<< m_nodePool.memoryUsage() << endl << endl;
		return ss.str();
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