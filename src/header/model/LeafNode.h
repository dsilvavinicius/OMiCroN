#ifndef LEAF_NODE_H
#define LEAF_NODE_H

#include "Point.h"
#include "OctreeNode.h"

namespace model
{
	template< typename MortonCode >
	class OctreeNode;
	
	template< typename MortonCode, typename Contents >
	class LeafNode : public OctreeNode< MortonCode >
	{
	public:
		bool isLeaf() const;
		void setContents( const Contents& contents );
		shared_ptr< Contents > getContents() const;
		
		template< typename M, typename C >
		friend ostream& operator<<( ostream& out, const LeafNode< M, C >& node );
	private:
		shared_ptr< Contents > m_contents;
	};
	
	template < typename MortonCode, typename Contents >
	inline bool LeafNode< MortonCode, Contents >::isLeaf() const
	{
		return true;
	}
	
	template < typename MortonCode, typename Contents >
	inline void LeafNode< MortonCode, Contents >::setContents( const Contents& contents )
	{
		m_contents = make_shared< Contents>(contents);
	}
	
	template < typename MortonCode, typename Contents >
	inline shared_ptr< Contents > LeafNode< MortonCode, Contents >::getContents() const
	{
		return m_contents;
	}
	
	template < typename MortonCode, typename Contents >
	ostream& operator<<(ostream& out, const LeafNode< MortonCode, Contents >& node)
	{
		out << "Points Leaf Node: " << endl
			<< *node.getContents();

		return out;
	}
	
	//===========
	// Type sugar
	//===========
	
	template< typename MortonCode, typename Contents >
	using LeafNodePtr = shared_ptr< LeafNode< MortonCode, Contents > >;
	
	template< typename Contents >
	using ShallowLeafNode = LeafNode< ShallowMortonCode, Contents >;
}

#endif