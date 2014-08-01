#ifndef LEAF_NODE_H
#define LEAF_NODE_H

#include "Point.h"
#include "OctreeNode.h"

namespace model
{
	template < typename MortonPrecision, typename Float, typename Vec3,
		typename Contents >
	class LeafNode : public OctreeNode<MortonPrecision, Float, Vec3>
	{
	public:
		bool isLeaf();
		void setContents(const Contents& contents);
		shared_ptr< Contents > getContents();
	private:
		shared_ptr< Contents > m_contents;
	};
	
	template < typename MortonPrecision, typename Float, typename Vec3,
		typename Contents >
	bool LeafNode<MortonPrecision, Float, Vec3, Contents>::isLeaf()
	{
		return true;
	}
	
	template < typename MortonPrecision, typename Float, typename Vec3,
		typename Contents >
	void LeafNode<MortonPrecision, Float, Vec3, Contents>::setContents(
		const Contents& contents)
	{
		m_contents = make_shared< Contents>(contents);
	}
	
	template < typename MortonPrecision, typename Float, typename Vec3,
		typename Contents >
	shared_ptr< Contents > LeafNode<MortonPrecision, Float, Vec3, Contents>::
		getContents()
	{
		return m_contents;
	}
	
	/** Leaf node with a list of points. */
	template <typename MortonPrecision, typename Float, typename Vec3>
	using PointsLeafNode = LeafNode< MortonPrecision, Float, Vec3, vector< PointPtr<Vec3> > >;
	
	/** PointsLeafNode smart pointer. */
	template <typename MortonPrecision, typename Float, typename Vec3>
	using PointsLeafNodePtr = shared_ptr< PointsLeafNode< MortonPrecision, Float, Vec3 > >;
}

#endif