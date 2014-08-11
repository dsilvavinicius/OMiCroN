#ifndef INNER_NODE_H
#define INNER_NODE_H

#include <glm/glm.hpp>
#include "OctreeNode.h"

using namespace glm;

namespace model
{
	template <typename MortonPrecision, typename Float, typename Vec3>
	class OctreeNode;
	
	template < typename MortonPrecision, typename Float, typename Vec3, typename Contents >
	class InnerNode;
	
	/** Inner node with one vertex as LOD. */
	template < typename MortonPrecision, typename Float, typename Vec3>
	using LODInnerNode = InnerNode<MortonPrecision, Float, Vec3, Point< Float, Vec3 > >;
	
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
	
	template < typename MortonPrecision, typename Float, typename Vec3, typename Contents >
	class InnerNode : public OctreeNode<MortonPrecision, Float, Vec3>
	{
	public:
		bool isLeaf() const;
		void setContents(const Contents& contents);
		shared_ptr< Contents > getContents() const;
		
		template <typename M, typename F, typename V>
		friend ostream& operator<<(ostream& out, const LODInnerNode< M, F, V >& node);
		
		template <typename M, typename F, typename V>
		friend ostream& operator<<(ostream& out, const PerFaceLODInnerNode< M, F, V >& node);
		
	private:
		shared_ptr< Contents > m_contents;
	};
	
	template < typename MortonPrecision, typename Float, typename Vec3, typename Contents>
	bool InnerNode<MortonPrecision, Float, Vec3, Contents>::isLeaf() const
	{
		return false;
	}
	
	template < typename MortonPrecision, typename Float, typename Vec3, typename Contents>
	void InnerNode<MortonPrecision, Float, Vec3, Contents>::setContents(
		const Contents& contents)
	{
		m_contents = make_shared< Contents >(contents);
	}
	
	template < typename MortonPrecision, typename Float, typename Vec3, typename Contents>
	shared_ptr< Contents > InnerNode<MortonPrecision, Float, Vec3, Contents>::
		getContents() const
	{
		return m_contents;
	}
	
	template <typename MortonPrecision, typename Float, typename Vec3>
	ostream& operator<<(ostream& out, const LODInnerNode< MortonPrecision, Float, Vec3 >& node)
	{
		out << "LOD Inner Node: " << endl << *node.getContents();
		return out;
	}
	
	template <typename MortonPrecision, typename Float, typename Vec3>
	ostream& operator<<(ostream& out, const PerFaceLODInnerNode< MortonPrecision, Float, Vec3 >& node)
	{
		out << "Per-Face LOD Inner Node: " << endl;
		for (PointPtr< Float, Vec3 > point : *node.getContents())
		{
			out << *point;
		}
		return out;
	}
}
	
#endif