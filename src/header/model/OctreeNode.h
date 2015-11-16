#ifndef OCTREE_NODE_H
#define OCTREE_NODE_H

#include <vector>
#include <glm/glm.hpp>
#include "Serializer.h"
#include "MemoryUtils.h"
#include "NodeReleaser.h"

using namespace std;
using namespace glm;

namespace model
{
	/** Simple octree node. */
	template< typename Contents >
	class OctreeNode
	{
	public:
		OctreeNode( const bool& isLeaf )
		: m_isLeaf( isLeaf ) {}
		
		~OctreeNode() { NodeReleaser::release( *this ); }
		
		void* operator new( size_t size );
		void* operator new[]( size_t size );
		void operator delete( void* p );
		void operator delete[]( void* p );
		
		/** Indicates the type of the node. */
		bool isLeaf() const { return m_isLeaf; }

		/** Sets the contents of this node. */
		void setContents( const Contents& contents ) { m_contents = contents; }
		
		/** Gets the contents of this node. */
		Contents& getContents() { return m_contents; }
		
		/** Gets the const contents of this node. */
		const Contents& getContents() const { return m_contents; }
		
		// TODO: make an operator<< instead since the method doesn't need to be a template anymore.
		/** Does the same as operator<< (however the compiler has bugs regarding template operator<< and is avoided here) */
		void output( ostream& out );
		
		/** Serializes the node. The form is:
		 *	bool flag true if the node is leaf, false otherwise;
		 *	serialization of content. */
		size_t serialize( byte** serialization ) const;
		
		/** Deserializes a byte sequence into an OctreeNode. The pointer is owned by the caller. */
		static OctreeNodePtr< Contents > deserialize( byte* serialization );
		
	private:
		Contents m_contents;
		bool m_isLeaf;
	};
	
	template< typename Contents >
	void* OctreeNode< Contents >::operator new( size_t size )
	{
		return ManagedAllocator< OctreeNode< Contents > >().allocate( 1 );
	}
	
	template< typename Contents >
	void* OctreeNode< Contents >::operator new[]( size_t size )
	{
		return ManagedAllocator< OctreeNode< Contents > >().allocate( size / sizeof( OctreeNode< Contents > ) );
	}
	
	template< typename Contents >
	void OctreeNode< Contents >::operator delete( void* p )
	{
		ManagedAllocator< OctreeNode< Contents > >().deallocate(
			static_cast< typename ManagedAllocator< OctreeNode< Contents > >::pointer >( p ), 1
		);
	}
	
	template< typename Contents >
	void OctreeNode< Contents >::operator delete[]( void* p )
	{
		ManagedAllocator< OctreeNode< Contents > >().deallocate(
			static_cast< typename ManagedAllocator< OctreeNode< Contents > >::pointer >( p ), 2
		);
	}
	
	template < typename Contents >
	void OctreeNode< Contents >::output( ostream& out )
	{
		out << "Points Leaf Node: " << endl << getContents();
	}
	
	template< typename Contents >
	inline size_t OctreeNode< Contents >::serialize( byte** serialization ) const
	{
		byte* content;
		size_t contentSize = Serializer::serialize( getContents(), &content );
		
		bool flag = isLeaf();
		size_t flagSize = sizeof( bool );
		size_t nodeSize = flagSize + contentSize;
		
		*serialization = new byte[ nodeSize ];
		byte* tempPtr = *serialization;
		memcpy( tempPtr, &flag, flagSize );
		tempPtr += flagSize;
		memcpy( tempPtr, content, contentSize );
		
		Serializer::dispose( content );
		
		return nodeSize;
	}
	
	template< typename Contents >
	inline OctreeNodePtr< Contents > OctreeNode< Contents >::deserialize( byte* serialization )
	{
		bool flag;
		size_t flagSize = sizeof( bool );
		memcpy( &flag, serialization, flagSize );
		byte* tempPtr = serialization + flagSize;
		
		auto node = makeManaged< OctreeNode< Contents > >( flag );
		Contents contents;
		Serializer::deserialize( tempPtr, contents );
		node->setContents( contents );
		return node;
	}
	
	template< typename Contents >
	using OctreeNodePtr = shared_ptr< OctreeNode< Contents > >;
}

#endif