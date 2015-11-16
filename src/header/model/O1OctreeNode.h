#ifndef O1_OCTREE_NODE_H
#define O1_OCTREE_NODE_H

#include <memory>
#include "Array.h"
#include "OctreeMapTypes.h"
#include "ConcurrentOctreeMapTypes.h"

using namespace std;

namespace model
{
	/** Octree node that provide all operations in O(1) time. Expects that sibling groups are allocated continuously in
	 * memory. Each node is responsable only of children resources.
	 * @param Contents is the element type of the array member. */
	template< typename Contents, typename Alloc = TbbAllocator< Contents > >
	class O1OctreeNode
	{
	public:
		using NodeArray = Array< O1OctreeNode, Alloc >;
		
		O1OctreeNode( NodeArray& contents, bool isLeaf )
		: m_contents( contents ),
		m_isLeaf( isLeaf ),
		m_parent( nullptr ),
		m_children( 0 )
		{}
		
		void* operator new( size_t size );
		void* operator new[]( size_t size );
		void operator delete( void* p );
		void operator delete[]( void* p );
		
		Array< Contents >& getContents() { return m_contents; }
		
		/** Gets a pointer for the parent of this node. */
		O1OctreeNode* parent() const { return m_parent; }
		
		/** Gets a pointer for the left sibling of this node. The caller must know if the pointer is dereferenceable. */
		O1OctreeNode* leftSibling() const { return this + 1; }
		
		/** Gets a pointer for the right sibling of this node. The caller must know if the pointer is dereferenceable. */
		O1OctreeNode* rightSibling() const { return this - 1; };
		
		/** Gets the pointer for left-most child of this node. */
		NodeArray& child() const { return m_children; }
		
		bool isLeaf() const { return m_isLeaf; }
		
		/** Sets parent pointer. */
		void setParent( const O1OctreeNode* parent ) { m_parent = parent; }
		
		/** Sets the array of children. */
		void setChildren( const NodeArray& children )
		{
			m_children = children;
		}
		
		void setChildren( NodeArray&& children )
		{
			m_children = children;
		}
		
		template< typename C >
		friend ostream& operator<<( ostream& out, const O1OctreeNode< C >& node );
		
		size_t serialize( byte** serialization ) const;
		
		static O1OctreeNode deserialize( byte* serialization );
		
	private:
		Array< Contents > m_contents;
		
		// CACHE INVARIANT. Parent pointer. Is always corrent after octree bottom-up creation, since octree cache release
		// is bottom-up.
		O1OctreeNode* m_parent; 
		
		// CACHE VARIANT. Array with all children of this nodes. Can be empty even if the node is not leaf, since
		// children can be released from octree cache.
		NodeArray m_children;
		
		// CACHE INVARIANT. Indicates if the node is leaf.
		bool m_isLeaf; 
	};
	
	template< typename Contents, typename Alloc >
	inline void* O1OctreeNode< Contents, Alloc >::operator new( size_t size )
	{
		return Alloc().allocate( 1 );
	}
	
	template< typename Contents, typename Alloc >
	inline void* O1OctreeNode< Contents, Alloc >::operator new[]( size_t size )
	{
		return Alloc().allocate( size / sizeof( O1OctreeNode< Contents > ) );
	}
	
	template< typename Contents, typename Alloc >
	inline void O1OctreeNode< Contents, Alloc >::operator delete( void* p )
	{
		Alloc().deallocate( static_cast< typename ManagedAllocator< O1OctreeNode< Contents > >::pointer >( p ), 1 );
	}
	
	template< typename Contents, typename Alloc >
	inline void O1OctreeNode< Contents, Alloc >::operator delete[]( void* p )
	{
		Alloc().deallocate( static_cast< typename ManagedAllocator< O1OctreeNode< Contents > >::pointer >( p ), 2 );
	}
	
	template< typename Contents, typename Alloc >
	inline size_t O1OctreeNode< Contents, Alloc >::serialize( byte** serialization ) const
	{
		byte* content;
		size_t contentSize = Serializer::serialize( getContents(), &content );
		
		size_t flagSize = sizeof( bool );
		size_t nodeSize = flagSize + contentSize;
		
		*serialization = new byte[ nodeSize ];
		byte* tempPtr = *serialization;
		memcpy( tempPtr, &m_isLeaf, flagSize );
		tempPtr += flagSize;
		memcpy( tempPtr, content, contentSize );
		
		Serializer::dispose( content );
		
		return nodeSize;
	}
	
	template< typename Contents, typename Alloc >
	inline O1OctreeNode< Contents, Alloc > O1OctreeNode< Contents, Alloc >::deserialize( byte* serialization )
	{
		bool flag;
		size_t flagSize = sizeof( bool );
		memcpy( &flag, serialization, flagSize );
		byte* tempPtr = serialization + flagSize;
		
		Array< Contents > contents;
		Serializer::deserialize( tempPtr, contents );
		
		auto node = O1OctreeNode< Contents, Alloc >( contents, flag );
		return node;
	}
}

#endif