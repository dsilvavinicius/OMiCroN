#ifndef OCTREE_NODE_H
#define OCTREE_NODE_H

#include <vector>
#include <glm/glm.hpp>
#include "InnerNode.h"
#include "LeafNode.h"
#include "Serializer.h"
#include "ExtendedPoint.h"

using namespace std;
using namespace glm;

namespace model
{
	template< typename MortonCode, typename Point >
	class Octree;
	
	// TODO: Eliminates the reinterpret_casts on this class.
	/** Base class for octree nodes. */
	template< typename MortonCode >
	class OctreeNode
	{
		using IndexVector = vector< unsigned long >;
	public:
		/** Indicates the type of the node. */
		virtual bool isLeaf() const = 0;
		
		/** Sets the contents of this node. Implies reinterpret_cast downawards hierarchy. */
		template< typename Contents >
		void setContents( const Contents& contents );
		
		/** Gets the contents of this node. Implies reinterpret_cast downawards hierarchy. */
		template< typename Contents >
		shared_ptr< Contents > getContents() const;
		
		template< typename M, typename C >
		friend ostream& operator<<( ostream& out, OctreeNode< M >& node );
		
		/** Serializes the node. The form is:
		 *	bool flag true if the node is leaf, false otherwise;
		 *	serialization of content. */
		template< typename Contents >
		size_t serialize( byte** serialization ) const;
		
		/** Deserializes a byte sequence into an OctreeNode. The pointer is owned by the caller, who needs to delete it
		 *	later on. */
		template< typename Contents >
		static OctreeNode* deserialize( byte* serialization );
	};
	
	template< typename MortonCode >
	template< typename Contents >
	inline void OctreeNode< MortonCode >::setContents( const Contents& contents )
	{
		if( isLeaf() )
		{
			LeafNode< MortonCode, Contents >* node =
				reinterpret_cast< LeafNode< MortonCode, Contents > >( this );
			
			node->setContents( contents );
		}
		else
		{
			InnerNode< MortonCode, Contents >* node =
				reinterpret_cast< LeafNode< MortonCode, Contents > >( this );
			
			node->setContents(contents);
		}
	}
		
	template< typename MortonCode >
	template< typename Contents >
	inline shared_ptr< Contents > OctreeNode< MortonCode >::getContents() const
	{
		if( isLeaf() )
		{
			const LeafNode< MortonCode, Contents >* node =
				reinterpret_cast< const LeafNode< MortonCode, Contents >* >( this );
			
			return node->getContents();
		}
		else
		{
			const InnerNode< MortonCode, Contents >* node =
				reinterpret_cast< const InnerNode< MortonCode, Contents >* >( this );
			
			return node->getContents();
		}
	}
	
	template< typename MortonCode, typename Contents >
	ostream& operator<<( ostream& out, OctreeNode< MortonCode >& node )
	{
		if( node.isLeaf() )
		{
			auto* leaf = reinterpret_cast< LeafNode< MortonCode, Contents >* >( &node );
			out << *leaf;
		}
		else
		{
			auto* inner = reinterpret_cast< InnerNode< MortonCode, Contents >* >( &node );
			out << *inner;
		}
		
		return out;
	}
	
	template< typename MortonCode >
	template< typename Contents >
	inline size_t OctreeNode< MortonCode >::serialize( byte** serialization ) const
	{
		byte* content;
		size_t contentSize = Serializer::serialize( *getContents< Contents >(), &content );
		
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
	
	template< typename MortonCode >
	template< typename Contents >
	inline OctreeNode< MortonCode >* OctreeNode< MortonCode >::deserialize( byte* serialization )
	{
		bool flag;
		size_t flagSize = sizeof( bool );
		memcpy( &flag, serialization, flagSize );
		byte* tempPtr = serialization + flagSize;
		
		if( flag )
		{
			auto node = new LeafNode< MortonCode, Contents >();
			Contents contents;
			Serializer::deserialize( tempPtr, contents );
			node->setContents( contents );
			return node;
		}
		else
		{
			auto node = new InnerNode< MortonCode, Contents >();
			Contents contents;
			Serializer::deserialize( tempPtr, contents );
			node->setContents( contents );
			return node;
		}
	}
	
	template< typename MortonCode >
	using OctreeNodePtr = shared_ptr< OctreeNode< MortonCode > >;
	
	using ShallowOctreeNode = OctreeNode< ShallowMortonCode >;
	
	using ShallowOctreeNodePtr = shared_ptr< ShallowOctreeNode >;
	
	using MediumOctreeNode = OctreeNode< MediumMortonCode >;
	
	using MediumOctreeNodePtr = shared_ptr< MediumOctreeNode >;
	
	//using DeepOctreeNode = OctreeNode< DeepMortonCode >;
	
	//using DeepOctreeNodePtr = shared_ptr< DeepOctreeNode >;
}

#endif