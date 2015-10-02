#ifndef INNER_NODE_H
#define INNER_NODE_H

#include <glm/glm.hpp>
#include "OctreeNode.h"
#include "MortonCode.h"
#include "NodeReleaser.h"
#include "MemoryManager.h"

using namespace glm;

namespace model
{
	template< typename Contents >
	class InnerNode
	: public OctreeNode
	{
	public:
		void* operator new( size_t size );
		void* operator new[]( size_t size );
		void operator delete( void* p );
		void operator delete[]( void* p );
		~InnerNode();
		bool isLeaf() const;
		void setContents(const Contents& contents);
		Contents& getContents();
		const Contents& getContents() const;
		
		void output( ostream& out );
		
	private:
		Contents m_contents;
	};
	
	template< typename Contents >
	void* InnerNode< Contents >::operator new( size_t size )
	{
		return SingletonMemoryManager::instance().allocInner();
	}
	
	template< typename Contents >
	void* InnerNode< Contents >::operator new[]( size_t size )
	{
		return SingletonMemoryManager::instance().allocInnerArray( size );
	}
	
	template< typename Contents >
	void InnerNode< Contents >::operator delete( void* p )
	{
		SingletonMemoryManager::instance().deallocInner( p );
	}
	
	template< typename Contents >
	void InnerNode< Contents >::operator delete[]( void* p )
	{
		SingletonMemoryManager::instance().deallocInnerArray( p );
	}
	
	template< typename Contents>
	inline InnerNode< Contents >::~InnerNode()
	{
		NodeReleaser::releaseInner( *this );
	}
	
	template< typename Contents>
	inline bool InnerNode< Contents >::isLeaf() const
	{
		return false;
	}
	
	template< typename Contents>
	inline void InnerNode< Contents >::setContents( const Contents& contents )
	{
		m_contents = contents;
	}
	
	template< typename Contents>
	inline Contents& InnerNode< Contents >::getContents()
	{
		return m_contents;
	}
	
	template< typename Contents>
	inline const Contents& InnerNode< Contents >::getContents() const
	{
		return m_contents;
	}
	
	template< typename Contents >
	void InnerNode< Contents >::output( ostream& out )
	{
		out << "Inner Node: " << endl << getContents();
	}
	
	//===========
	// Type sugar
	//===========
	
	template< typename Contents >
	using InnerNodePtr = shared_ptr< InnerNode< Contents > >;
}
	
#endif