#ifndef O1_OCTREE_NODE_H
#define O1_OCTREE_NODE_H

#include <memory>
#include "tucano.hpp"
#include "Array.h"
#include "GpuAllocStatistics.h"
#include "splat_renderer/surfel_cloud.h"

using namespace std;
using namespace Tucano;

namespace model
{
	/** Octree node that provide all operations in O(1) time. Expects that sibling groups are allocated continuously in
	 * memory. Each node is responsable only for children resources.
	 * @param Contents is the element type of the array member.
	 * @param ContentsAlloc is the allocator used for Contents type. The default allocator provides multithreaded
	 * environment support. */
	template< typename Contents, typename ContentsAlloc = TbbAllocator< Contents > >
	class O1OctreeNode
	{
	public:
		using ContentsArray = Array< Contents, ContentsAlloc >;
		using NodeAlloc = typename ContentsAlloc:: template rebind< O1OctreeNode >::other;
		using NodeArray = Array< O1OctreeNode, NodeAlloc >;
		
		enum LoadState
		{
			LOADED = 0x0,
			UNLOADED = 0x1,
			PENDING = 0x2,
		};
		
		/** Initializes and empty unusable node. */
		O1OctreeNode()
		: m_contents(),
		m_isLeaf( false ),
		m_parent( nullptr ),
		m_children(),
		m_loadState( UNLOADED )
		{}
		
		O1OctreeNode( const ContentsArray& contents, const bool isLeaf )
		: m_contents( contents ),
		m_isLeaf( isLeaf ),
		m_parent( nullptr ),
		m_children(),
		m_loadState( UNLOADED )
		{}
		
		O1OctreeNode( ContentsArray&& contents, const bool isLeaf )
		: m_contents( std::move( contents ) ),
		m_isLeaf( isLeaf ),
		m_parent( nullptr ),
		m_children(),
		m_loadState( UNLOADED )
		{}
		
		/** IMPORTANT: parent pointer is not deeply copied, since the node has responsibility only over its own children
		 * resources. Mesh GPU data is not copied. */
		O1OctreeNode( const O1OctreeNode& other )
		: m_contents( other.m_contents ),
		m_children( other.m_children ),
		m_parent( other.m_parent ),
		m_isLeaf( other.m_isLeaf ),
		m_loadState( UNLOADED )
		{}
		
		~O1OctreeNode();
		
		/** IMPORTANT: parent pointer is not deeply copied, since the node has responsibility only over its own children
		 * resources. Mesh GPU data is not copied. */
		O1OctreeNode& operator=( const O1OctreeNode& other )
		{
			m_contents = other.m_contents;
			m_children = other.m_children;
			m_parent = other.m_parent;
			m_isLeaf = other.m_isLeaf;
			m_loadState = UNLOADED;
			
			return *this;
		}
		
		/** Move ctor. */
		O1OctreeNode( O1OctreeNode&& other )
		: m_contents( std::move( other.m_contents ) ),
		m_children( std::move( other.m_children ) ),
		m_cloud( std::move( other.m_cloud ) ),
		m_parent( other.m_parent ),
		m_isLeaf( other.m_isLeaf ),
		m_loadState( other.m_loadState.load() )
		{
			other.m_parent = nullptr;
		}
		
		/** Move assignment. */
		O1OctreeNode& operator=( O1OctreeNode&& other )
		{
			m_contents = std::move( other.m_contents );
			m_children = std::move( other.m_children );
			m_cloud = std::move( other.m_cloud );
			m_parent = other.m_parent;
			m_isLeaf = other.m_isLeaf;
			m_loadState = other.m_loadState.load();
			
			other.m_parent = nullptr;
			
			return *this;
		}
		
		void* operator new( size_t size );
		void* operator new[]( size_t size );
		void operator delete( void* p );
		void operator delete[]( void* p );
		
		const ContentsArray& getContents() const { return m_contents; }
		
		/** Gets a pointer for the parent of this node. */
		O1OctreeNode* parent() const { return m_parent; }
		
		/** Gets a pointer for the left sibling of this node. The caller must know if the pointer is dereferenceable. */
		O1OctreeNode* leftSibling() { return this - 1; }
		
		/** Gets a pointer for the right sibling of this node. The caller must know if the pointer is dereferenceable. */
		O1OctreeNode* rightSibling() { return this + 1; };
		
		/** Gets the pointer for left-most child of this node. */
		NodeArray& child() { return m_children; }
		const NodeArray& child() const { return m_children; }
		
		bool isLeaf() const { return m_isLeaf; }
		
		void setContents( ContentsArray&& contents )
		{
			m_contents.~ContentsArray();
			m_contents = std::move( contents );
		}
		
		/** Sets parent pointer. */
		void setParent( O1OctreeNode* parent ) { m_parent = parent; }
		
		/** Sets the array of children. */
		void setChildren( const NodeArray& children )
		{
			m_children.~NodeArray();
			m_children = children;
		}
		
		void setChildren( NodeArray&& children )
		{
			m_children.~NodeArray();
			m_children = std::move( children );
		}
		
		/** Transforms the node into a leaf, releasing all child nodes. */
		void turnLeaf()
		{
			m_isLeaf = true;
			m_children.clear();
		}
		
		const SurfelCloud& cloud() const { return m_cloud; }
		
		SurfelCloud& cloud() { return m_cloud; }
		
		void loadCloud( const SurfelCloud& cloud )
		{
			m_cloud = cloud;
			m_loadState = LoadState::LOADED;
		}
		
		void unloadCloud()
		{
			m_cloud = SurfelCloud();
			m_loadState = LoadState::UNLOADED;
		}
		
		LoadState loadState() const { return LoadState( m_loadState.load() ); }
		
		void setPendingCloud() { m_loadState = LoadState::PENDING; } 
		
		template< typename C >
		friend ostream& operator<<( ostream& out, const O1OctreeNode< C >& node );
		
		size_t serialize( byte** serialization ) const;
		
		static O1OctreeNode deserialize( byte* serialization );
		
	private:
		SurfelCloud m_cloud;
		
		ContentsArray m_contents;
		
		// CACHE INVARIANT. Parent pointer. Is always current after octree bottom-up creation, since octree cache release
		// is bottom-up.
		O1OctreeNode* m_parent; 
		
		// CACHE VARIANT. Array with all children of this nodes. Can be empty even if the node is not leaf, since
		// children can be released from octree cache.
		NodeArray m_children;
		
		atomic_char m_loadState;
		
		// CACHE INVARIANT. Indicates if the node is leaf.
		bool m_isLeaf;
	};
	
	template< typename Contents, typename ContentsAlloc >
	inline O1OctreeNode< Contents, ContentsAlloc >::~O1OctreeNode()
	{
		m_parent = nullptr;
		
		if( m_loadState == LOADED )
		{
			ulong pointSize = GpuAllocStatistics::pointSize();
			GpuAllocStatistics::notifyDealloc( pointSize * m_contents.size() );
		}
	}
	
	template< typename Contents, typename ContentsAlloc >
	inline void* O1OctreeNode< Contents, ContentsAlloc >::operator new( size_t size )
	{
		return NodeAlloc().allocate( 1 );
	}
	
	template< typename Contents, typename ContentsAlloc >
	inline void* O1OctreeNode< Contents, ContentsAlloc >::operator new[]( size_t size )
	{
		return NodeAlloc().allocate( size / sizeof( O1OctreeNode< Contents > ) );
	}
	
	template< typename Contents, typename ContentsAlloc >
	inline void O1OctreeNode< Contents, ContentsAlloc >::operator delete( void* p )
	{
		NodeAlloc().deallocate( static_cast< typename NodeAlloc::pointer >( p ), 1 );
	}
	
	template< typename Contents, typename ContentsAlloc >
	inline void O1OctreeNode< Contents, ContentsAlloc >::operator delete[]( void* p )
	{
		NodeAlloc().deallocate( static_cast< typename NodeAlloc::pointer >( p ), 2 );
	}
	
	template< typename Contents, typename ContentsAlloc >
	inline size_t O1OctreeNode< Contents, ContentsAlloc >::serialize( byte** serialization ) const
	{
		byte* content;
		size_t contentSize = getContents().serialize( &content );
		
		size_t flagSize = sizeof( bool );
		size_t nodeSize = flagSize + contentSize;
		
		*serialization = Serializer::newByteArray( nodeSize );
		byte* tempPtr = *serialization;
		memcpy( tempPtr, &m_isLeaf, flagSize );
		tempPtr += flagSize;
		memcpy( tempPtr, content, contentSize );
		
		Serializer::dispose( content );
		
		return nodeSize;
	}
	
	template< typename Contents, typename ContentsAlloc >
	inline O1OctreeNode< Contents, ContentsAlloc > O1OctreeNode< Contents, ContentsAlloc >
	::deserialize( byte* serialization )
	{
		bool flag;
		size_t flagSize = sizeof( bool );
		memcpy( &flag, serialization, flagSize );
		byte* tempPtr = serialization + flagSize;
		
		auto contents = Array< Contents >::deserialize( tempPtr );
		
		auto node = O1OctreeNode< Contents, ContentsAlloc >( contents, flag );
		return node;
	}
	
	template< typename C >
	ostream& operator<<( ostream& out, const O1OctreeNode< C >& node )
	{
		out << "points: " << endl << node.m_contents << endl
			<< "is leaf? " << node.m_isLeaf << endl << endl;
			
		return out;
	}
}

#endif