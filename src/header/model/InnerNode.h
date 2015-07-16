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
	template< typename MortonCode >
	class OctreeNode;
	
	// TODO: MortonCode seems to be unnecessary here.
	template< typename MortonCode, typename Contents >
	class InnerNode : public OctreeNode< MortonCode >
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
		
		template< typename M, typename C >
		friend ostream& operator<<( ostream& out, const InnerNode< M, C >& node );
		
	private:
		Contents m_contents;
	};
	
	template< typename MortonCode, typename Contents >
	void* InnerNode< MortonCode, Contents >::operator new( size_t size )
	{
		return MemoryManager::instance().allocateNode();
	}
	
	template< typename MortonCode, typename Contents >
	void* InnerNode< MortonCode, Contents >::operator new[]( size_t size )
	{
		throw logic_error( "InnerNode::operator new[] is unsupported." );
	}
	
	template< typename MortonCode, typename Contents >
	void InnerNode< MortonCode, Contents >::operator delete( void* p )
	{
		MemoryManager::instance().deallocateNode( p );
	}
	
	template< typename MortonCode, typename Contents >
	void InnerNode< MortonCode, Contents >::operator delete[]( void* p )
	{
		throw logic_error( "InnerNode::operator delete[] is unsupported." );
	}
	
	template < typename MortonCode, typename Contents>
	inline InnerNode< MortonCode, Contents >::~InnerNode()
	{
		NodeReleaser::releaseInner( *this );
	}
	
	template < typename MortonCode, typename Contents>
	inline bool InnerNode< MortonCode, Contents >::isLeaf() const
	{
		return false;
	}
	
	template < typename MortonCode, typename Contents>
	inline void InnerNode< MortonCode, Contents >::setContents( const Contents& contents )
	{
		m_contents = contents;
	}
	
	template < typename MortonCode, typename Contents>
	inline Contents& InnerNode< MortonCode, Contents >::getContents()
	{
		return m_contents;
	}
	
	template < typename MortonCode, typename Contents>
	inline const Contents& InnerNode< MortonCode, Contents >::getContents() const
	{
		return m_contents;
	}
	
	template< typename MortonCode, typename Contents >
	ostream& operator<<( ostream& out, const InnerNode< MortonCode, Contents >& node )
	{
		out << "Inner Node: " << endl << *node.getContents();
		return out;
	}
	
	//===========
	// Type sugar
	//===========
	
	template< typename MortonCode, typename Contents >
	using InnerNodePtr = shared_ptr< InnerNode< MortonCode, Contents > >;
	
	template< typename Contents >
	using ShallowInnerNode = InnerNode< ShallowMortonCode, Contents >;
	
	template< typename Contents >
	using ShallowInnerNodePtr = shared_ptr< ShallowInnerNode< Contents > >;
}
	
#endif