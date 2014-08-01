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
	
	/** Leaf node with a list of points. */
	template <typename MortonPrecision, typename Float, typename Vec3>
	using PointsLeafNode = LeafNode<MortonPrecision, Float, Vec3,
		shared_ptr< vector< PointPtr<Vec3> > > >;
	
	/** PointsLeafNode smart pointer. */
	template <typename MortonPrecision, typename Float, typename Vec3>
	using PointsLeafNodePtr = shared_ptr<
		PointsLeafNode< MortonPrecision, Float, Vec3 > >;
}

#endif