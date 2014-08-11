#ifndef LEAF_NODE_H
#define LEAF_NODE_H

#include "Point.h"
#include "OctreeNode.h"

namespace model
{
	template <typename MortonPrecision, typename Float, typename Vec3>
	class OctreeNode;
	
	template < typename MortonPrecision, typename Float, typename Vec3,
		typename Contents >
	class LeafNode;
	
	/** Leaf node with a list of points. */
	template <typename MortonPrecision, typename Float, typename Vec3>
	using PointsLeafNode = LeafNode< MortonPrecision, Float, Vec3, PointVector< Float, Vec3 > >;
	
	/** PointsLeafNode smart pointer. */
	template <typename MortonPrecision, typename Float, typename Vec3>
	using PointsLeafNodePtr = shared_ptr< PointsLeafNode< MortonPrecision, Float, Vec3 > >;
	
	template < typename MortonPrecision, typename Float, typename Vec3,
		typename Contents >
	class LeafNode : public OctreeNode< MortonPrecision, Float, Vec3 >
	{
	public:
		bool isLeaf() const;
		void setContents(const Contents& contents);
		shared_ptr< Contents > getContents() const;
		
		template <typename M, typename F, typename V>
		friend ostream& operator<<(ostream& out, const PointsLeafNode< M, F, V >& node);
	private:
		shared_ptr< Contents > m_contents;
	};
	
	template < typename MortonPrecision, typename Float, typename Vec3, typename Contents >
	bool LeafNode<MortonPrecision, Float, Vec3, Contents>::isLeaf() const
	{
		return true;
	}
	
	template < typename MortonPrecision, typename Float, typename Vec3, typename Contents >
	void LeafNode<MortonPrecision, Float, Vec3, Contents>::setContents(
		const Contents& contents)
	{
		m_contents = make_shared< Contents>(contents);
	}
	
	template < typename MortonPrecision, typename Float, typename Vec3, typename Contents >
	shared_ptr< Contents > LeafNode<MortonPrecision, Float, Vec3, Contents>::
		getContents() const
	{
		return m_contents;
	}
	
	template < typename MortonPrecision, typename Float, typename Vec3 >
	ostream& operator<<(ostream& out, const PointsLeafNode< MortonPrecision, Float, Vec3 >& node)
	{
		out << "Points Leaf Node: " << endl;
		for (PointPtr< Float, Vec3 > point : *node.getContents())
		{
			out << *point;
		}
		return out;
	}
}

#endif