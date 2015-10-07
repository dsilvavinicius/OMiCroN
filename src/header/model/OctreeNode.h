#ifndef OCTREE_NODE_H
#define OCTREE_NODE_H

#include <vector>
#include <glm/glm.hpp>
#include "Serializer.h"
#include "ExtendedPoint.h"
#include "MemoryUtils.h"

using namespace std;
using namespace glm;

namespace model
{
	template< typename Contents >
	class InnerNode;
	
	template< typename Contents >
	class LeafNode;
	
	// TODO: Eliminates the reinterpret_casts on this class.
	/** Base class for octree nodes. */
	class OctreeNode
	{
	public:
		virtual ~OctreeNode() {}
		
		/** Indicates the type of the node. */
		virtual bool isLeaf() const = 0;
		
		/** Sets the contents of this node. Implies reinterpret_cast downawards hierarchy. */
		template< typename Contents >
		void setContents( const Contents& contents );
		
		/** Gets the contents of this node. Implies reinterpret_cast downawards hierarchy. */
		template< typename Contents >
		Contents& getContents();
		
		/** Gets the const contents of this node. Implies reinterpret_cast downawards hierarchy. */
		template< typename Contents >
		const Contents& getContents() const;
		
		/** Does the same as operator<< (however the compiler has bugs regarding template operator<< and is avoided here) */
		template< typename Contents >
		void output( ostream& out );
		
		/** Serializes the node. The form is:
		 *	bool flag true if the node is leaf, false otherwise;
		 *	serialization of content. */
		template< typename Contents >
		size_t serialize( byte** serialization ) const;
		
		/** Deserializes a byte sequence into an OctreeNode. The pointer is owned by the caller, who needs to delete it
		 *	later on. */
		template< typename Contents >
		static OctreeNodePtr deserialize( byte* serialization );
	};
	
	template< typename Contents >
	inline void OctreeNode::setContents( const Contents& contents )
	{
		if( isLeaf() )
		{
			LeafNode< Contents >* node =
				reinterpret_cast< LeafNode< Contents >* >( this );
			
			node->setContents( contents );
		}
		else
		{
			InnerNode< Contents >* node =
				reinterpret_cast< InnerNode< Contents >* >( this );
			
			node->setContents( contents );
		}
	}
		
	template< typename Contents >
	inline Contents& OctreeNode::getContents()
	{
		if( isLeaf() )
		{
			LeafNode< Contents >* node = reinterpret_cast< LeafNode< Contents >* >( this );
			return node->getContents();
		}
		else
		{
			InnerNode< Contents >* node = reinterpret_cast< InnerNode< Contents >* >( this );
			return node->getContents();
		}
	}
	
	template< typename Contents >
	inline const Contents& OctreeNode::getContents() const
	{
		if( isLeaf() )
		{
			const LeafNode< Contents >* node =
				reinterpret_cast< const LeafNode< Contents >* >( this );
			return node->getContents();
		}
		else
		{
			const InnerNode< Contents >* node =
				reinterpret_cast< const InnerNode< Contents >* >( this );
			return node->getContents();
		}
	}
	
	template< typename Contents >
	void OctreeNode::output( ostream& out )
	{
		if( isLeaf() )
		{
			auto* leaf = reinterpret_cast< LeafNode< Contents >* >( this );
			leaf->output( out );
		}
		else
		{
			auto* inner = reinterpret_cast< InnerNode< Contents >* >( this );
			inner->output( out );
		}
	}
	
	template< typename Contents >
	inline size_t OctreeNode::serialize( byte** serialization ) const
	{
		byte* content;
		size_t contentSize = Serializer::serialize( getContents< Contents >(), &content );
		
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
	inline OctreeNodePtr OctreeNode::deserialize( byte* serialization )
	{
		bool flag;
		size_t flagSize = sizeof( bool );
		memcpy( &flag, serialization, flagSize );
		byte* tempPtr = serialization + flagSize;
		
		if( flag )
		{
			auto node = makeManaged< LeafNode< Contents > >();
			Contents contents;
			Serializer::deserialize( tempPtr, contents );
			node->setContents( contents );
			return node;
		}
		else
		{
			auto node = makeManaged< InnerNode< Contents > >();
			Contents contents;
			Serializer::deserialize( tempPtr, contents );
			node->setContents( contents );
			return node;
		}
	}
	
	using OctreeNodePtr = shared_ptr< OctreeNode >;
}

#endif