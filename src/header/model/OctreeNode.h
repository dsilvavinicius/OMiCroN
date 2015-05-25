#ifndef OCTREE_NODE_H
#define OCTREE_NODE_H

#include <vector>
#include <glm/glm.hpp>
#include "Point.h"
#include "InnerNode.h"
#include "LeafNode.h"
#include "Serializer.h"

using namespace std;
using namespace glm;

namespace model
{
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	class Octree;
	
	// TODO: Eliminates the reinterpret_casts on this class.
	/** Base class for octree nodes. */
	template< typename MortonPrecision, typename Float, typename Vec3 >
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
		
		/** Returns two vertices that defines the boundaries box of this node (minimum, maximum). */
		pair< Vec3, Vec3 > getBoundaries() const;
		
		template< typename M, typename F, typename V, typename C >
		friend ostream& operator<<( ostream& out, OctreeNode< M, F, V >& node );
		
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
	
	template< typename MortonPrecision, typename Float, typename Vec3 >
	template< typename Contents >
	inline void OctreeNode< MortonPrecision, Float, Vec3 >::setContents( const Contents& contents )
	{
		if( isLeaf() )
		{
			LeafNode< MortonPrecision, Float, Vec3, Contents >* node =
				reinterpret_cast< LeafNode< MortonPrecision, Float, Vec3, Contents > >( this );
			
			node->setContents( contents );
		}
		else
		{
			InnerNode< MortonPrecision, Float, Vec3, Contents >* node =
				reinterpret_cast< LeafNode< MortonPrecision, Float, Vec3, Contents > >( this );
			
			node->setContents(contents);
		}
	}
		
	template< typename MortonPrecision, typename Float, typename Vec3 >
	template< typename Contents >
	inline shared_ptr< Contents > OctreeNode< MortonPrecision, Float, Vec3 >::getContents() const
	{
		if( isLeaf() )
		{
			const LeafNode< MortonPrecision, Float, Vec3, Contents >* node =
				reinterpret_cast< const LeafNode< MortonPrecision, Float, Vec3, Contents >* >( this );
			
			return node->getContents();
		}
		else
		{
			const InnerNode< MortonPrecision, Float, Vec3, Contents >* node =
				reinterpret_cast< const InnerNode< MortonPrecision, Float, Vec3, Contents >* >( this );
			
			return node->getContents();
		}
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Contents >
	ostream& operator<<( ostream& out, OctreeNode< MortonPrecision, Float, Vec3 >& node )
	{
		if( node.isLeaf() )
		{
			auto* leaf = reinterpret_cast< LeafNode< MortonPrecision, Float, Vec3, Contents >* >( &node );
			out << *leaf;
		}
		else
		{
			auto* inner = reinterpret_cast< InnerNode< MortonPrecision, Float, Vec3, Contents >* >( &node );
			out << *inner;
		}
		
		return out;
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3 >
	template< typename Contents >
	inline size_t OctreeNode< MortonPrecision, Float, Vec3 >::serialize( byte** serialization ) const
	{
		byte* content;
		size_t contentSize = Serializer::serialize( *getContents< Contents >(), &content );
		
		bool flag = isLeaf();
		size_t flagSize = sizeof( bool );
		size_t nodeSize = flagSize + contentSize;
		
		*serialization = new byte[ nodeSize ];
		memcpy( *serialization, &flag, flagSize );
		memcpy( *serialization + flagSize, content, contentSize );
		
		Serializer::dispose( content );
		
		return nodeSize;
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3 >
	template< typename Contents >
	inline OctreeNode< MortonPrecision, Float, Vec3 >* OctreeNode< MortonPrecision, Float, Vec3 >::deserialize(
		byte* serialization )
	{
		bool flag;
		size_t flagSize = sizeof( bool );
		memcpy( &flag, serialization, flagSize );
		byte* tempPtr = serialization + flagSize;
		
		if( flag )
		{
			auto node = new LeafNode< MortonPrecision, Float, Vec3, Contents >();
			Contents contents;
			Serializer::deserialize( tempPtr, contents );
			node->setContents( contents );
			return node;
		}
		else
		{
			auto node = new InnerNode< MortonPrecision, Float, Vec3, Contents >();
			Contents contents;
			Serializer::deserialize( tempPtr, contents );
			node->setContents( contents );
			return node;
		}
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3 >
	using OctreeNodePtr = shared_ptr< OctreeNode< MortonPrecision, Float, Vec3 > >;
	
	template< typename Float, typename Vec3 >
	using ShallowOctreeNode = OctreeNode< unsigned int, Float, Vec3 >;
	
	template< typename Float, typename Vec3 >
	using ShallowOctreeNodePtr = shared_ptr< ShallowOctreeNode< Float, Vec3 > >;
	
	template< typename Float, typename Vec3 >
	using MediumOctreeNode = OctreeNode< unsigned long, Float, Vec3 >;
	
	template< typename Float, typename Vec3 >
	using MediumOctreeNodePtr = shared_ptr< MediumOctreeNode< Float, Vec3 > >;
	
	template< typename Float, typename Vec3 >
	using DeepOctreeNode = OctreeNode< unsigned long long, Float, Vec3 >;
	
	template< typename Float, typename Vec3 >
	using DeepOctreeNodePtr = shared_ptr< DeepOctreeNode< Float, Vec3 > >;
}

#endif