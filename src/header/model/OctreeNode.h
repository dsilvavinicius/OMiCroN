#ifndef OCTREE_NODE_H
#define OCTREE_NODE_H

#include <vector>
#include <glm/glm.hpp>
#include "Point.h"

using namespace std;
using namespace glm;

namespace model
{
	class Octree;
	
	/** Base class for octree nodes. Can be an inner node or a leaf node. */
	class OctreeNode
	{
	public:
		/** Indicates the type of the node. */
		virtual bool isLeaf() = 0;
		/** Casts to inner node. Throws an exception if is a leaf.*/
		InnerNode asInner();
		/** Casts to leaf node. Throws an exception if is an inner.*/
		LeafNode asLeaf();
	protected:
		shared_ptr<Octree> m_octree;
	};
	
	using OctreeNodePtr = shared_ptr<OctreeNode>;
}

#endif