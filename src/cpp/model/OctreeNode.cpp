#include "OctreeNode.h"

namespace model
{
	template <typename MortonPrecision, typename Float, typename Vec3,
		typename Contents>
	void OctreeNode::setContents(Contents contents)
	{
		if (isLeaf())
		{
			LeafNode<MortonPrecision, Contents>* node =
				dynamic_cast< LeafNode<MortonPrecision, Contents> >(this);
			
			node->setContents(contents);
		}
		else
		{
			InnerNode<MortonPrecision, Contents>* node =
				dynamic_cast< LeafNode<MortonPrecision, Contents> >(this);
			
			node->setContents(contents);
		}
	}
		
	template <typename MortonPrecision, typename Float, typename Vec3,
		typename Contents>
	T OctreeNode::getContents()
	{
		if (isLeaf())
		{
			LeafNode<MortonPrecision, Contents>* node =
				dynamic_cast< LeafNode<MortonPrecision, Contents> >(this);
			
			return node->getContents();
		}
		else
		{
			InnerNode<MortonPrecision, Contents>* node =
				dynamic_cast< LeafNode<MortonPrecision, Contents> >(this);
			
			return node->getContents();
		}
	}
}