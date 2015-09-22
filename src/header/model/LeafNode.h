#ifndef LEAF_NODE_H
#define LEAF_NODE_H

#include "Point.h"
#include "OctreeNode.h"
#include "NodeReleaser.h"
#include "MemoryManager.h"

namespace model
{
	template< typename MortonCode >
	class OctreeNode;
	
	// TODO: MortonCode seems to be unnecessary here.
	template< typename MortonCode, typename Contents >
	class LeafNode : public OctreeNode< MortonCode >
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
	
	template< typename MortonCode, typename Contents >
	void* LeafNode< MortonCode, Contents >::operator new( size_t size )
	{
		return SingletonMemoryManager::instance().allocLeaf();
	}
	
	template< typename MortonCode, typename Contents >
	void* LeafNode< MortonCode, Contents >::operator new[]( size_t size )
	{
		return SingletonMemoryManager::instance().allocLeafArray( size );
	}
	
	template< typename MortonCode, typename Contents >
	void LeafNode< MortonCode, Contents >::operator delete( void* p )
	{
		SingletonMemoryManager::instance().deallocLeaf( p );
	}
	
	template< typename MortonCode, typename Contents >
	void LeafNode< MortonCode, Contents >::operator delete[]( void* p )
	{
		SingletonMemoryManager::instance().deallocLeafArray( p );
	}
	
	template < typename MortonCode, typename Contents >
	inline LeafNode< MortonCode, Contents >::~LeafNode()
	{
		//cout << "Destructing leaf" << endl;
		NodeReleaser::releaseLeaf( *this );
	}
	
	template < typename MortonCode, typename Contents >
	inline bool LeafNode< MortonCode, Contents >::isLeaf() const
	{
		return true;
	}
	
	template < typename MortonCode, typename Contents >
	inline void LeafNode< MortonCode, Contents >::setContents( const Contents& contents )
	{
		m_contents = contents;
	}
	
	template < typename MortonCode, typename Contents >
	inline Contents& LeafNode< MortonCode, Contents >::getContents()
	{
		return m_contents;
	}
	
	template < typename MortonCode, typename Contents >
	inline const Contents& LeafNode< MortonCode, Contents >::getContents() const
	{
		return m_contents;
	}
	
	template < typename MortonCode, typename Contents >
	void LeafNode< MortonCode, Contents >::output( ostream& out )
	{
		out << "Points Leaf Node: " << endl << getContents();
	}
	
	//===========
	// Type sugar
	//===========
	
	template< typename MortonCode, typename Contents >
	using LeafNodePtr = shared_ptr< LeafNode< MortonCode, Contents > >;
	
	template< typename Contents >
	using ShallowLeafNode = LeafNode< ShallowMortonCode, Contents >;
	
	template< typename Contents >
	using ShallowLeafNodePtr = shared_ptr< ShallowLeafNode< Contents > >;
	
	template< typename Contents >
	using MediumLeafNode = LeafNode< MediumMortonCode, Contents >;
	
	template< typename Contents >
	using MediumLeafNodePtr = shared_ptr< MediumLeafNode< Contents > >;
}

#endif