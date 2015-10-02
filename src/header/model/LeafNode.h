#ifndef LEAF_NODE_H
#define LEAF_NODE_H

#include "Point.h"
#include "OctreeNode.h"
#include "NodeReleaser.h"
#include "MemoryManager.h"

namespace model
{
	template< typename Contents >
	class LeafNode
	: public OctreeNode
	{
	public:
		void* operator new( size_t size );
		void* operator new[]( size_t size );
		void operator delete( void* p );
		void operator delete[]( void* p );
		
		~LeafNode();
		bool isLeaf() const;
		void setContents( const Contents& contents );
		Contents& getContents();
		const Contents& getContents() const;
		
		void output( ostream& out );
	
	private:
		Contents m_contents;
	};
	
	template< typename Contents >
	void* LeafNode< Contents >::operator new( size_t size )
	{
		return SingletonMemoryManager::instance().allocLeaf();
	}
	
	template< typename Contents >
	void* LeafNode< Contents >::operator new[]( size_t size )
	{
		return SingletonMemoryManager::instance().allocLeafArray( size );
	}
	
	template< typename Contents >
	void LeafNode< Contents >::operator delete( void* p )
	{
		SingletonMemoryManager::instance().deallocLeaf( p );
	}
	
	template< typename Contents >
	void LeafNode< Contents >::operator delete[]( void* p )
	{
		SingletonMemoryManager::instance().deallocLeafArray( p );
	}
	
	template < typename Contents >
	inline LeafNode< Contents >::~LeafNode()
	{
		//cout << "Destructing leaf" << endl;
		NodeReleaser::releaseLeaf( *this );
	}
	
	template < typename Contents >
	inline bool LeafNode< Contents >::isLeaf() const
	{
		return true;
	}
	
	template < typename Contents >
	inline void LeafNode< Contents >::setContents( const Contents& contents )
	{
		m_contents = contents;
	}
	
	template < typename Contents >
	inline Contents& LeafNode< Contents >::getContents()
	{
		return m_contents;
	}
	
	template < typename Contents >
	inline const Contents& LeafNode< Contents >::getContents() const
	{
		return m_contents;
	}
	
	template < typename Contents >
	void LeafNode< Contents >::output( ostream& out )
	{
		out << "Points Leaf Node: " << endl << getContents();
	}
	
	//===========
	// Type sugar
	//===========
	
	template< typename Contents >
	using LeafNodePtr = shared_ptr< LeafNode< Contents > >;
}

#endif