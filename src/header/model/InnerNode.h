#ifndef INNER_NODE_H
#define INNER_NODE_H

#include <glm/glm.hpp>
#include "OctreeNode.h"

using namespace glm;

namespace model
{
	template < typename MortonPrecision, typename Float, typename Vec3,
		typename Contents >
	class InnerNode : OctreeNode<MortonPrecision, Float, Vec3>
	{
	public:
		bool isLeaf();
		void setContents(const Contents& contents);
		shared_ptr< Contents > getContents();
	private:
		shared_ptr< Contents > m_contents;
	};
	
	template < typename MortonPrecision, typename Float, typename Vec3,
		typename Contents>
	bool InnerNode<MortonPrecision, Float, Vec3, Contents>::isLeaf()
	{
		return false;
	}
	
	template < typename MortonPrecision, typename Float, typename Vec3,
		typename Contents>
	void InnerNode<MortonPrecision, Float, Vec3, Contents>::setContents(
		const Contents& contents)
	{
		m_contents = make_shared< Contents >(contents);
	}
	
	template < typename MortonPrecision, typename Float, typename Vec3,
		typename Contents>
	shared_ptr< Contents > InnerNode<MortonPrecision, Float, Vec3, Contents>::
		getContents()
	{
		return m_contents;
	}
	
	/** Inner node with one vertex as LOD. */
	template < typename MortonPrecision, typename Float, typename Vec3>
	using LODInnerNode = InnerNode<MortonPrecision, Float, Vec3, Point<Vec3> >;
	
	/** Smart pointer for LODInnerNode. */ 
	template < typename MortonPrecision, typename Float, typename Vec3>
	using LODInnerNodePtr = shared_ptr< LODInnerNode< MortonPrecision, Float, Vec3 > >;
	
	/** Inner node with LOD as one vertex per cube face. */
	template < typename MortonPrecision, typename Float, typename Vec3>
	using PerFaceLODInnerNode = InnerNode< MortonPrecision, Float, Vec3, vector<Vec3> >;
	
	/** Smart pointer for PerFaceLODInnerNode. */ 
	template < typename MortonPrecision, typename Float, typename Vec3>
	using PerFaceLODInnerNodePtr = shared_ptr< PerFaceLODInnerNode<
		MortonPrecision, Float, Vec3 > >;
}
	
#endif