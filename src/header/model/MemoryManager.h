#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <vector>
#include <sstream>
#include "IMemoryPool.h"
#include "IMemoryManager.h"

namespace model
{
	/** Skeletal base implementation of a SingletonMemoryManager. This implementation does not initialize members, what
	 * is left for derived classes to do.
	 * @param Morton is the morton code type.
	 * @param Point is the point Type.
	 * @param Inner is the inner node type.
	 * @param Leaf is the leaf node type.
	 */
	template< typename Morton, typename Point, typename Inner, typename Leaf >
	class MemoryManager
	: public SingletonMemoryManager
	{
	public:
		virtual ~MemoryManager() = 0;
		
		void* allocMorton() override;
		
		void* allocPoint() override;
		
		void* allocInner() override;
		
		void* allocLeaf() override;
		
		void* allocMortonArray(  const size_t& size ) override;
		
		void* allocPointArray( const size_t& size ) override;
		
		void* allocInnerArray( const size_t& size ) override;
		
		void* allocLeafArray( const size_t& size ) override;
		
		void deallocMorton( void* p ) override;
		
		void deallocPoint( void* p ) override;
		
		void deallocInner( void* p ) override;
		
		void deallocLeaf( void* p ) override;
		
		void deallocMortonArray( void* p ) override;
		
		void deallocPointArray( void* p ) override;
		
		void deallocInnerArray( void* p ) override;
		
		void deallocLeafArray( void* p ) override;
		
		size_t usedMemory() const override;
		
		size_t maxAllowedMem() const override;
		
		bool hasEnoughMemory( const float& percentageThreshold ) const override;
		
		template< typename T >
		struct TypeWrapper{};
		
		IMemoryPool< Morton >& getPool( const struct TypeWrapper< Morton >& type );
		
		IMemoryPool< Point >& getPool( const struct TypeWrapper< Point >& type );
		
		IMemoryPool< Inner >& getPool( const struct TypeWrapper< Inner >& type );
		
		IMemoryPool< Leaf >& getPool( const struct TypeWrapper< Leaf >& type );
		
		string toString() const override;
		
		size_t numMortonBlocks() const;
		
		size_t numPointBlocks() const;
		
		size_t numInnerBlocks() const;
		
		size_t numLeafBlocks() const;
		
	protected:
		IMemoryPool< Morton >* m_mortonPool;
		IMemoryPool< Point >* m_pointPool;
		IMemoryPool< Inner >* m_innerPool;
		IMemoryPool< Leaf >* m_leafPool;
		size_t m_maxAllowedMem;
	};
	
	template< typename Morton, typename Point, typename Inner, typename Leaf >
	MemoryManager< Morton, Point, Inner, Leaf >::~MemoryManager()
	{
		delete m_mortonPool;
		delete m_pointPool;
		delete m_innerPool;
		delete m_leafPool;
	}
	
	template< typename Morton, typename Point, typename Inner, typename Leaf >
	inline void* MemoryManager< Morton, Point, Inner, Leaf >::allocMorton()
	{
		return m_mortonPool->allocate();
	}
	
	template< typename Morton, typename Point, typename Inner, typename Leaf >
	inline void* MemoryManager< Morton, Point, Inner, Leaf >::allocPoint()
	{
		return m_pointPool->allocate();
	}
	
	template< typename Morton, typename Point, typename Inner, typename Leaf >
	inline void* MemoryManager< Morton, Point, Inner, Leaf >::allocInner()
	{
		return m_innerPool->allocate();
	}
	
	template< typename Morton, typename Point, typename Inner, typename Leaf >
	inline void* MemoryManager< Morton, Point, Inner, Leaf >::allocLeaf()
	{
		return m_leafPool->allocate();
	}
	
	template< typename Morton, typename Point, typename Inner, typename Leaf >
	void* MemoryManager< Morton, Point, Inner, Leaf >::allocMortonArray( const size_t& size )
	{
		return m_mortonPool->allocateArray( size );
	}
	
	template< typename Morton, typename Point, typename Inner, typename Leaf >
	void* MemoryManager< Morton, Point, Inner, Leaf >::allocPointArray( const size_t& size )
	{
		return m_pointPool->allocateArray( size );
	}
	
	template< typename Morton, typename Point, typename Inner, typename Leaf >
	void* MemoryManager< Morton, Point, Inner, Leaf >::allocInnerArray( const size_t& size )
	{
		return m_innerPool->allocateArray( size );
	}
	
	template< typename Morton, typename Point, typename Inner, typename Leaf >
	void* MemoryManager< Morton, Point, Inner, Leaf >::allocLeafArray( const size_t& size )
	{
		return m_leafPool->allocateArray( size );
	}
	
	template< typename Morton, typename Point, typename Inner, typename Leaf >
	inline void MemoryManager< Morton, Point, Inner, Leaf >::deallocMorton( void* p )
	{
		m_mortonPool->deallocate( static_cast< Morton* >( p ) );
	}
	
	template< typename Morton, typename Point, typename Inner, typename Leaf >
	inline void MemoryManager< Morton, Point, Inner, Leaf >::deallocPoint( void* p )
	{
		m_pointPool->deallocate( static_cast< Point* >( p ) );
	}
	
	template< typename Morton, typename Point, typename Inner, typename Leaf >
	inline void MemoryManager< Morton, Point, Inner, Leaf >::deallocInner( void* p )
	{
		m_innerPool->deallocate( static_cast< Inner* >( p ) );
	}
	
	template< typename Morton, typename Point, typename Inner, typename Leaf >
	inline void MemoryManager< Morton, Point, Inner, Leaf >::deallocLeaf( void* p )
	{
		m_leafPool->deallocate( static_cast< Leaf* >( p ) );
	}
	
	template< typename Morton, typename Point, typename Inner, typename Leaf >
	void MemoryManager< Morton, Point, Inner, Leaf >::deallocMortonArray( void* p )
	{
		m_mortonPool->deallocateArray( static_cast< Morton* >( p ) );
	}
	
	template< typename Morton, typename Point, typename Inner, typename Leaf >
	void MemoryManager< Morton, Point, Inner, Leaf >::deallocPointArray( void* p )
	{
		m_pointPool->deallocateArray( static_cast< Point* >( p ) );
	}
	
	template< typename Morton, typename Point, typename Inner, typename Leaf >
	void MemoryManager< Morton, Point, Inner, Leaf >::deallocInnerArray( void* p )
	{
		m_innerPool->deallocateArray( static_cast< Inner* >( p ) );
	}
	
	template< typename Morton, typename Point, typename Inner, typename Leaf >
	void MemoryManager< Morton, Point, Inner, Leaf >::deallocLeafArray( void* p )
	{
		m_leafPool->deallocateArray( static_cast< Leaf* >( p ) );
	}
	
	template< typename Morton, typename Point, typename Inner, typename Leaf >
	inline size_t MemoryManager< Morton, Point, Inner, Leaf >::usedMemory() const
	{
		return 	m_mortonPool->memoryUsage() + m_pointPool->memoryUsage() + m_innerPool->memoryUsage()
				+ m_leafPool->memoryUsage();
	}
	
	template< typename Morton, typename Point, typename Inner, typename Leaf >
	size_t MemoryManager< Morton, Point, Inner, Leaf >::maxAllowedMem() const
	{
		return m_maxAllowedMem;
	}
	
	template< typename Morton, typename Point, typename Inner, typename Leaf >
	inline bool MemoryManager< Morton, Point, Inner, Leaf >::hasEnoughMemory( const float& percentageThreshold ) const
	{
		return ( m_maxAllowedMem - usedMemory() ) > float( m_maxAllowedMem * percentageThreshold );
	}
	
	template< typename Morton, typename Point, typename Inner, typename Leaf >
	inline IMemoryPool< Morton >& MemoryManager< Morton, Point, Inner, Leaf >
	::getPool( const struct TypeWrapper< Morton >& type )
	{
		return m_mortonPool;
	}
	
	template< typename Morton, typename Point, typename Inner, typename Leaf >
	inline IMemoryPool< Point >& MemoryManager< Morton, Point, Inner, Leaf >
	::getPool( const struct TypeWrapper< Point >& type )
	{
		return m_pointPool;
	}
	
	template< typename Morton, typename Point, typename Inner, typename Leaf >
	inline IMemoryPool< Inner >& MemoryManager< Morton, Point, Inner, Leaf >
	::getPool( const struct TypeWrapper< Inner >& type )
	{
		return m_innerPool;
	}
	
	template< typename Morton, typename Point, typename Inner, typename Leaf >
	inline IMemoryPool< Leaf >& MemoryManager< Morton, Point, Inner, Leaf >
	::getPool( const struct TypeWrapper< Leaf >& type )
	{
		return m_leafPool;
	}
	
	template< typename Morton, typename Point, typename Inner, typename Leaf >
	string MemoryManager< Morton, Point, Inner, Leaf >::toString() const
	{
		stringstream ss;
		ss 	<< "Morton used blocks: " << m_mortonPool->usedBlocks() << " Used memory: " << m_mortonPool->memoryUsage() << endl
			<< "Point used blocks: " << m_pointPool->usedBlocks() << " Used memory: " << m_pointPool->memoryUsage() << endl
			<< "Inner used blocks: " << m_innerPool->usedBlocks() << " Used memory: " << m_innerPool->memoryUsage() << endl
			<< "Leaf used blocks: " << m_leafPool->usedBlocks() << " Used memory: " << m_leafPool->memoryUsage() << endl << endl;
		return ss.str();
	}
	
	template< typename Morton, typename Point, typename Inner, typename Leaf >
	size_t MemoryManager< Morton, Point, Inner, Leaf >::numMortonBlocks() const
	{
		return m_mortonPool->getNumBlocks();
	}
	
	template< typename Morton, typename Point, typename Inner, typename Leaf >
	size_t MemoryManager< Morton, Point, Inner, Leaf >::numPointBlocks() const
	{
		return m_pointPool->getNumBlocks();
	}
	
	template< typename Morton, typename Point, typename Inner, typename Leaf >
	size_t MemoryManager< Morton, Point, Inner, Leaf >::numInnerBlocks() const
	{
		return m_innerPool->getNumBlocks();
	}
	
	template< typename Morton, typename Point, typename Inner, typename Leaf >
	size_t MemoryManager< Morton, Point, Inner, Leaf >::numLeafBlocks() const
	{
		return m_leafPool->getNumBlocks();
	}
}

#endif