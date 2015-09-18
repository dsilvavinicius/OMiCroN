#include "MemoryManager.h"
#include <MortonCode.h>
#include <LeafNode.h>

namespace model
{
	MemoryManager::MemoryManager( const ulong& nShallowMorton, const ulong& nMediumMorton, const ulong& nPoints,
								  const ulong& nExtendedPoints, const ulong& nNodes )
	{
		m_shallowMortonPool.createPool( nShallowMorton );
		m_mediumMortonPool.createPool( nMediumMorton );
		m_pointPool.createPool( nPoints );
		m_extendedPointPool.createPool( nExtendedPoints );
		m_nodePool.createPool( nNodes );
		
		m_maxAllowedMem = 	nShallowMorton * sizeof( ShallowMortonCode ) + nMediumMorton * sizeof( MediumMortonCode )
							+ nPoints * sizeof( Point ) + nExtendedPoints * sizeof( ExtendedPoint )
							+ nNodes * sizeof( ShallowLeafNode< PointVector > );
	}
	
	void* MemoryManager::allocate( const MANAGED_TYPE_FLAG& type )
	{
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
	
	void MemoryManager::deallocate( void* p, const MANAGED_TYPE_FLAG& type )
	{
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
	
	size_t MemoryManager::usedMemory() const
	{
		return 	m_shallowMortonPool.memoryUsage() + m_mediumMortonPool.memoryUsage() + m_pointPool.memoryUsage()
				+ m_extendedPointPool.memoryUsage() + m_nodePool.memoryUsage();
	}
	
	string MemoryManager::toString() const
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
	
	size_t MemoryManager::numBlocks( const MANAGED_TYPE_FLAG& type ) const
	{
		switch( type )
		{
			case SHALLOW_MORTON		: return m_shallowMortonPool.getNumBlocks();
			case MEDIUM_MORTON		: return m_mediumMortonPool.getNumBlocks();
			case POINT				: return m_pointPool.getNumBlocks();
			case EXTENDED_POINT		: return m_extendedPointPool.getNumBlocks();
			case NODE				: return m_nodePool.getNumBlocks();
			default					: throw logic_error( "Unknowm nanaged type" );
		}
	}
}