#ifndef INNER_NODE_H
#define INNER_NODE_H

#include <glm/glm.hpp>
#include "OctreeNode.h"

using namespace glm;

namespace model
{
	template <typename MortonPrecision, typename Contents>
	class InnerNode : OctreeNode<MortonPrecision>
	{
	public:
		bool isLeaf();
		void setContents(Contents contents);
		Contents getContents();
	private:
		Contents m_contents;
	};
	
	/** Inner node with one vertex as LOD. */
	template <typename MortonPrecision>
	using LODInnerNode = InnerNode<MortonPrecision, vec3>;
	
	/** Smart pointer for LODInnerNode. */ 
	template <typename MortonPrecision>
	using LODInnerNodePtr = shared_ptr< LODInnerNode<MortonPrecision> >;
	
	/** Inner node with LOD as one vertex per cube face. */
	template <typename MortonPrecision>
	using PerFaceLODInnerNode = InnerNode<MortonPrecision, vector<vec3> >;
	
	/** Smart pointer for PerFaceLODInnerNode. */ 
	template <typename MortonPrecision>
	using PerFaceLODInnerNodePtr = shared_ptr< PerFaceLODInnerNode<MortonPrecision> >;
}
	
#endif