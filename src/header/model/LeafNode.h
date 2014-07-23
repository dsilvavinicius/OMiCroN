#ifndef LEAF_NODE_H
#define LEAF_NODE_H

namespace model
{
	template <typename T>
	class LeafNode : OctreeNode
	{
	public:
		bool isLeaf();
		vector<T>& getValues();
	private:
		vector<T> values;
	};
	
	using LeafNodePtr = shared_ptr<LeafNode>;
}

#endif