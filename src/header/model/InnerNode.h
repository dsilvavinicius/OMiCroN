#ifndef INNER_NODE_H
#define INNER_NODE_H

#include <glm/glm.hpp>
#include "OctreeNode.h"

using namespace glm;

namespace model
{
	template <typename T>
	class InnerNode : OctreeNode
	{
	public:
		bool isLeaf();
		T& getValue();
	private:
		T value;
	};
	
	using LODInnerNode = InnerNode<vec3>
	
	using InnerNodePtr = shared_ptr<InnerNode>;
}
	
#endif