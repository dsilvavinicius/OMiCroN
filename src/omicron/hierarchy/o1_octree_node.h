#ifndef O1_OCTREE_NODE_H
#define O1_OCTREE_NODE_H

#include <memory>
#include <stxxl/vector>
#include "tucano/tucano.hpp"
#include "omicron/renderer/splat_renderer/surfel_cloud.h"
#include "omicron/hierarchy/hierarchy_creation_log.h"
#include "omicron/util/stack_trace.h"
#include "omicron/memory/global_malloc.h"
#include "omicron/hierarchy/octree_dimensions.h"

// #define CTOR_DEBUG
// #define LOADING_DEBUG

namespace omicron::hierarchy
{
    using namespace Tucano;
//     using namespace util;
    
	/** Out-of-core octree node, which accesses its point data by indices. Each node has a range in a shared out-of-core vector for indices. The points are stores in another shared out-of-core vector.
	 */
	template< typename Morton >
	class O1OctreeNode
	{
	public:
		using NodeAlloc = TbbAllocator< O1OctreeNode >;
		using NodeVector = Array< O1OctreeNode >;
		using IndexVector = std::vector< ulong, TbbAllocator< ulong > >;
		
		/** Initializes an empty unusable node. */
		O1OctreeNode();

		/** Ctor to build an O1OctreeNode when child, parent and indices are not known yet. Gpu cloud is also not init. The constructed node can also be considered a placeholder for another node. */
		O1OctreeNode( const Morton morton );

		/** Ctor to build a leaf O1OctreeNode when parent are not known yet. Gpu cloud is also not init.*/
		O1OctreeNode( const Morton morton, const IndexVector& indices );

		/** Ctor to build an inner O1OctreeNode totally init except for the gpu cloud. */
		O1OctreeNode( const Morton morton, NodeVector&& children );

		/** Ctor to build a leaf O1OctreeNode totally init except for the gpu cloud. */
		// O1OctreeNode( const Morton morton, O1OctreeNode* parent );
		
		
		
		O1OctreeNode( const O1OctreeNode& other ) = delete;
		
		~O1OctreeNode();
		
		O1OctreeNode& operator=( const O1OctreeNode& other ) = delete;
		
		/** Move ctor. */
		O1OctreeNode( O1OctreeNode&& other );
		
		/** Move assignment. */
		O1OctreeNode& operator=( O1OctreeNode&& other );
		
		/** Ctor to init from stream. Parent data */
		// O1OctreeNode( ifstream& input );

		void* operator new( size_t size );
		void* operator new[]( size_t size );
		void operator delete( void* p );
		void operator delete[]( void* p );
		
		const Morton& getMorton() const { return m_morton; }

		const uint size() const { return m_indexSize; } 

		/* const ContentsArray& getContents() const { return m_contents; }
		
		ContentsArray& getContents() { return m_contents; } */
		
		/** Gets a pointer for the parent of this node. */
		O1OctreeNode* parent() const { return m_parent; }
		
		/** Gets a pointer for the left sibling of this node. The caller must know if the pointer is dereferenceable. */
		O1OctreeNode* leftSibling() { return this - 1; }
		
		/** Gets a pointer for the right sibling of this node. The caller must know if the pointer is dereferenceable. */
		O1OctreeNode* rightSibling() { return this + 1; };
		
		/** Gets the pointer for left-most child of this node. */
		NodeVector& child() { return m_children; }
		const NodeVector& child() const { return m_children; }
		
		bool isLeaf() const { return m_isLeaf; }
		
		void setIndices();
		
		/** Sets parent pointer. */
		void setParent( O1OctreeNode* parent ) { m_parent = parent; }
		
		/** Sets the array of children. */
		void setChildren( const NodeVector& children );
		
		void setChildren( NodeVector&& children );
		
		/** Release the child nodes. The node is not turned into leaf. Useful to release memory momentarily. */
		// void releaseChildren() { m_children.clear(); }
		
		/** Transforms the node into a leaf, releasing all child nodes. */
		// void turnLeaf();
		
		bool empty() const { return m_indexSize == 0; }
		
		const SurfelCloud& cloud() const { return *m_cloud; }
		
		SurfelCloud& cloud() { return *m_cloud; }
		
		void loadInGpu();
		
		void unloadInGpu();
		
		bool isLoaded() const;
		
		/** @returns the number of nodes and the number of contents in the subtree rooted by this node. */
		pair< uint, uint > subtreeStatistics() const;
		
		/** @returns the number of nodes and the number of contents in the subtree rooted by the children of this node. */
		void childrenStatistics( const O1OctreeNode& node, pair< uint, uint >& stats ) const;
		
		string toString() const;
		
		// Binary persistence. Structure: | leaf flag | point data | children data |
		// void persist( ostream& out ) const;
		
		template< typename M >
		friend ostream& operator<<( ostream& out, const O1OctreeNode< M >& node );
		
		// size_t serialize( byte** serialization ) const;

		// static O1OctreeNode deserialize( byte* serialization );

	private:
		void setIndices( const IndexVector& indices );

		Morton m_morton;

		SurfelCloud* m_cloud;
		
		ulong m_indexOffset;
		uint m_indexSize;

		// CACHE INVARIANT. Parent pointer. Is always current after octree bottom-up creation, since octree cache release
		// is bottom-up.
		O1OctreeNode* m_parent; 
		
		// CACHE VARIANT. Array with all children of this nodes. Can be empty even if the node is not leaf, since
		// children can be released from octree cache.
		NodeVector m_children;
		
		// CACHE INVARIANT. Indicates if the node is leaf.
		bool m_isLeaf;
	};

	template< typename Morton >
	inline O1OctreeNode< Morton >::O1OctreeNode()
	: m_morton(),
	m_indexOffset( 0 ),
	m_indexSize( 0 ),
	m_isLeaf( false ),
	m_parent( nullptr ),
	m_children(),
	m_cloud( nullptr )
	{}

	template< typename Morton >
	inline O1OctreeNode< Morton >::O1OctreeNode( const Morton morton )
	: O1OctreeNode()
	{
		m_morton = morton;
	}

	template< typename Morton >
	inline O1OctreeNode< Morton >::O1OctreeNode( const Morton morton, const IndexVector& indices )
	: O1OctreeNode( morton )
	{
		m_indexSize = indices.size();
		m_isLeaf = true;

		setIndices( indices );
	}

	/* template< typename Morton >
	inline O1OctreeNode< Morton >::O1OctreeNode( const Morton morton, O1OctreeNode* parent )
	: O1OctreeNode( morton )
	{
		m_isLeaf = true;
		m_parent = parent;
	} */
		
	template< typename Morton >
	inline O1OctreeNode< Morton >::O1OctreeNode( const Morton morton, NodeVector&& children )
	: O1OctreeNode( morton )
	{
		setChildren( std::move( children ) );
		m_isLeaf = false;
	}

	template< typename Morton >
	inline O1OctreeNode< Morton >::O1OctreeNode( O1OctreeNode&& other )
	: m_indexOffset( other.m_indexOffset ),
	m_indexSize( other.m_indexSize ),
	m_children( std::move( other.m_children ) ),
	m_cloud( other.m_cloud ),
	m_parent( other.m_parent ),
	m_isLeaf( other.m_isLeaf )
	{
		other.m_morton = Morton();
		other.m_indexOffset = 0;
		other.m_indexSize = 0;
		other.m_parent = nullptr;
		other.m_cloud = nullptr;
	}
		
	template< typename Morton >
	inline O1OctreeNode< Morton >& O1OctreeNode< Morton >::operator=( O1OctreeNode&& other )
	{
		m_indexOffset = other.m_indexOffset;
		m_indexSize = other.m_indexSize;
		m_children = std::move( other.m_children );
		m_cloud = other.m_cloud;
		m_parent = other.m_parent;
		m_isLeaf = other.m_isLeaf;
		
		other.m_morton = Morton();
		other.m_indexOffset = 0;
		other.m_indexSize = 0;
		other.m_parent = nullptr;
		other.m_cloud = nullptr;
		
		return *this;
	}
	
	/* template< typename Morton >
	inline O1OctreeNode< Morton >::O1OctreeNode( ifstream& input )
	: m_cloud( nullptr ),
	m_parent( nullptr )
	{
		input.read( reinterpret_cast< char* >( &m_isLeaf ), sizeof( bool ) );
		m_contents = ContentsArray( input );
		m_children = NodeVector( input );
		
		for( O1OctreeNode& child : m_children )
		{
			child.setParent( this );
		}
	} */

	template< typename Morton >
	inline O1OctreeNode< Morton >::~O1OctreeNode()
	{
		m_parent = nullptr;
		if( m_cloud )
		{
			delete m_cloud;
			m_cloud = nullptr;
		}
	}
	
	template< typename Morton >
	inline void* O1OctreeNode< Morton >::operator new( size_t size )
	{
		return NodeAlloc().allocate( 1 );
	}
	
	template< typename Morton >
	inline void* O1OctreeNode< Morton >::operator new[]( size_t size )
	{
		return NodeAlloc().allocate( size / sizeof( O1OctreeNode< Morton > ) );
	}
	
	template< typename Morton >
	inline void O1OctreeNode< Morton >::operator delete( void* p )
	{
		NodeAlloc().deallocate( static_cast< typename NodeAlloc::pointer >( p ), 1 );
	}
	
	template< typename Morton >
	inline void O1OctreeNode< Morton >::operator delete[]( void* p )
	{
		NodeAlloc().deallocate( static_cast< typename NodeAlloc::pointer >( p ), 2 );
	}
	
	template< typename Morton >
	inline void O1OctreeNode< Morton >::setIndices()
	{
		IndexVector sample;

		for( const O1OctreeNode& node : m_children )
		{
			int nSamples = std::max( 1.f, node.size() * PARENT_POINTS_RATIO_VALUE );
			for( int i = 0; i < nSamples; ++i )
			{
				int chosenIdx = rand() % nSamples + node.m_indexOffset;
				
				sample.push_back( chosenIdx );
			}
		}

		setIndices( sample );
	}

	template< typename Morton >
	inline void O1OctreeNode< Morton >::setIndices( const IndexVector& indices )
	{
		m_indexOffset = ExtOctreeData::reserveIndices( indices.size() );
		ExtOctreeData::copy2External( indices, m_indexOffset );
	}

	template< typename Morton >
	inline void O1OctreeNode< Morton >::setChildren( const NodeVector& children )
	{
		m_children.clear();
		m_children = children;
	}

	template< typename Morton >
	inline void O1OctreeNode< Morton >::setChildren( NodeVector&& children )
	{
		m_children.clear();
		m_children = std::move( children );
	}

	/*template< typename Morton >
	inline void O1OctreeNode< Morton >::turnLeaf()
	{
		m_isLeaf = true;
		releaseChildren();
	} */

	template< typename Morton >
	inline void O1OctreeNode< Morton >::loadInGpu()
	{
		if( m_cloud == nullptr && GpuAllocStatistics::hasMemoryFor( m_indexSize ) )
		{
			m_cloud = new SurfelCloud( m_indexOffset, m_indexSize, m_morton.getLevel() );
			
			#ifdef LOADING_DEBUG
			{
				stringstream ss; ss << "Cloud prepared to load: " << m_cloud << endl << endl;
				HierarchyCreationLog::logDebugMsg( ss.str() );
			}
			#endif
		}
	}
	
	template< typename Morton >
	inline void O1OctreeNode< Morton >::unloadInGpu()
	{
		if( m_cloud != nullptr )
		{
			#ifdef LOADING_DEBUG
			{
				stringstream ss; ss << "Unloading cloud: " << m_cloud << endl << endl;
				HierarchyCreationLog::logDebugMsg( ss.str() );
			}
			#endif
			
			delete m_cloud;
			m_cloud = nullptr;
		}
	}
	
	template< typename Morton >
	inline bool O1OctreeNode< Morton >::isLoaded() const
	{
		if( m_cloud != nullptr )
		{
			if( m_cloud->loadStatus() == SurfelCloud::LOADED )
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}

	template< typename Morton >
	inline string O1OctreeNode< Morton >::toString() const
	{
		stringstream ss;
		ss
			<< "Addr: " << this << endl
			<< "Points: " << m_indexSize << endl
			<< "Parent: " << m_parent << endl
			<< "Children: " << m_children.size() << endl
			<< "Is leaf? " << m_isLeaf << endl
			<< "Load state: " << isLoaded() << endl
// 			<< "Cloud: " << endl << node.m_cloud
			;
		
		return ss.str();
	}
	
	// Binary persistence. Structure: | leaf flag | point data | children data |
	/* template< typename Morton >
	inline void O1OctreeNode< Morton >::persist( ostream& out ) const
	{
		out.write( reinterpret_cast< const char* >( &m_isLeaf ), sizeof( bool ) );
		m_contents.persist( out );
		m_children.persist( out );
	} */

	template< typename Morton >
	pair< uint, uint > O1OctreeNode< Morton >::subtreeStatistics() const
	{
		pair< uint, uint > subtreeStats( 1u, m_indexSize );
		childrenStatistics( *this, subtreeStats );
		
		return subtreeStats;
	}
	
	template< typename Morton >
	void O1OctreeNode< Morton >::childrenStatistics( const O1OctreeNode& node, pair< uint, uint >& stats ) const
	{
		stats.first += node.m_children.size();
		
		for( const O1OctreeNode& child : node.m_children )
		{
			stats.second += child.m_indexSize;
			childrenStatistics( child, stats );
		}
	}
	
	/* template< typename Morton >
	inline size_t O1OctreeNode< Morton >::serialize( byte** serialization ) const
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
	} */

	/* template< typename Morton >
	inline O1OctreeNode< Morton > O1OctreeNode< Morton >
	::deserialize( byte* serialization )
	{
		bool flag;
		size_t flagSize = sizeof( bool );
		memcpy( &flag, serialization, flagSize );
		byte* tempPtr = serialization + flagSize;
		
		auto contents = Array< Contents >::deserialize( tempPtr );
		
		auto node = O1OctreeNode< Contents, ContentsAlloc >( contents, flag );
		return node;
	} */

	template< typename Morton >
	ostream& operator<<( ostream& out, const O1OctreeNode< Morton >& node )
	{
		out << node.toString();
		
		return out;
	}
}

#undef CTOR_DEBUG
#undef LOADING_DEBUG

#endif
