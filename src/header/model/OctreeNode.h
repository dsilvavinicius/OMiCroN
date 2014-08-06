#ifndef OCTREE_NODE_H
#define OCTREE_NODE_H

#include <vector>
#include <glm/glm.hpp>
#include "Point.h"
#include "InnerNode.h"
#include "LeafNode.h"

using namespace std;
using namespace glm;

namespace model
{
	template <typename MortonPrecision, typename Float, typename Vec3>
	class Octree;
	
	/** Base class for octree nodes. */
	template <typename MortonPrecision, typename Float, typename Vec3>
	class OctreeNode
	{
	public:
		/** Indicates the type of the node. */
		virtual bool isLeaf() = 0;
		
		/** Sets the contents of this node. Implies dynamic_cast downawards hierarchy. */
		template <typename Contents>
		void setContents(const Contents& contents);
		
		/** Gets the contents of this node. Implies dynamic_cast downawards hierarchy. */
		template <typename Contents>
		shared_ptr< Contents > getContents();
	protected:
		shared_ptr< Octree< MortonPrecision, Float, Vec3 > > m_octree;
	};
	
	template <typename MortonPrecision, typename Float, typename Vec3>
	template <typename Contents>
	void OctreeNode< MortonPrecision, Float, Vec3 >::setContents (const Contents& contents)
	{
		if (isLeaf())
		{
			LeafNode< MortonPrecision, Float, Vec3, Contents >* node =
				dynamic_cast< LeafNode< MortonPrecision, Float, Vec3, Contents > >(this);
			
			node->setContents(contents);
		}
		else
		{
			InnerNode< MortonPrecision, Float, Vec3, Contents >* node =
				dynamic_cast< LeafNode< MortonPrecision, Float, Vec3, Contents > >(this);
			
			node->setContents(contents);
		}
	}
		
	template <typename MortonPrecision, typename Float, typename Vec3>
	template <typename Contents>
	shared_ptr< Contents > OctreeNode< MortonPrecision, Float, Vec3 >::getContents()
	{
		if (isLeaf())
		{
			LeafNode< MortonPrecision, Float, Vec3, Contents >* node =
				dynamic_cast< LeafNode< MortonPrecision, Float, Vec3, Contents >* >(this);
			
			return node->getContents();
		}
		else
		{
			InnerNode< MortonPrecision, Float, Vec3, Contents >* node =
				dynamic_cast< InnerNode< MortonPrecision, Float, Vec3, Contents >* >(this);
			
			return node->getContents();
		}
	}
	
	template <typename MortonPrecision, typename Float, typename Vec3>
	using OctreeNodePtr = shared_ptr< OctreeNode<MortonPrecision, Float, Vec3> >;
}

#endif