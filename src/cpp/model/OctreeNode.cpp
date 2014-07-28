#include "OctreeNode.h"

namespace model
{
	template <typename MortonPrecision, typename Contents>
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
		
	template <typename T>
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