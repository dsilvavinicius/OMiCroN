#ifndef OCTREE_NODE_H
#define OCTREE_NODE_H

#include <vector>
#include <glm/glm.hpp>
#include "Point.h"

using namespace std;
using namespace glm;

namespace model
{
	template <typename MortonPrecision>
	class Octree;
	
	/** Base class for octree nodes. */
	template <typename MortonPrecision>
	class OctreeNode
	{
	public:
		/** Indicates the type of the node. */
		virtual bool isLeaf() = 0;
		
		template <typename Contents>
		void setContents(Contents contents);
		
		template <typename Contents>
		Contents getContents();
	protected:
		shared_ptr< Octree<MortonPrecision> > m_octree;
	};
	
	template <typename MortonPrecision>
	using OctreeNodePtr = shared_ptr< OctreeNode<MortonPrecision> >;
}

#endif