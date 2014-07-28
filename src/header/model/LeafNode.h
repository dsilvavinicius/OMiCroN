#ifndef LEAF_NODE_H
#define LEAF_NODE_H

#include "Point.h"
#include "OctreeNode.h"

namespace model
{
	template <typename MortonPrecision, typename Contents>
	class LeafNode : public OctreeNode<MortonPrecision>
	{
	public:
		bool isLeaf();
		void setContents(Contents contents);
		Contents getContents();
	private:
		Contents m_contents;
	};
	
	/** Leaf node with a list of points. */
	template <typename MortonPrecision>
	using PointsLeafNode = LeafNode<MortonPrecision, vector<Point> >;
	
	/** PointsLeafNode smart pointer. */
	template <typename MortonPrecision>
	using PointsLeafNodePtr = shared_ptr< PointsLeafNode<MortonPrecision> >;
}

#endif