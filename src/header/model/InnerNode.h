#ifndef INNER_NODE_H
#define INNER_NODE_H

#include <glm/glm.hpp>
#include "OctreeNode.h"

using namespace glm;

namespace model
{
	template< typename MortonPrecision, typename Float, typename Vec3 >
	class OctreeNode;
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Contents >
	class InnerNode;
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Contents >
	using InnerNodePtr = shared_ptr< InnerNode< MortonPrecision, Float, Vec3, Contents > >;
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Contents >
	class InnerNode : public OctreeNode< MortonPrecision, Float, Vec3 >
	{
	public:
		bool isLeaf() const;
		void setContents(const Contents& contents);
		shared_ptr< Contents > getContents() const;
		
		template< typename M, typename F, typename V, typename C >
		friend ostream& operator<<( ostream& out, const InnerNodePtr< M, F, V, C >& node );
		
	private:
		shared_ptr< Contents > m_contents;
	};
	
	template < typename MortonPrecision, typename Float, typename Vec3, typename Contents>
	inline bool InnerNode<MortonPrecision, Float, Vec3, Contents>::isLeaf() const
	{
		return false;
	}
	
	template < typename MortonPrecision, typename Float, typename Vec3, typename Contents>
	inline void InnerNode<MortonPrecision, Float, Vec3, Contents>::setContents(
		const Contents& contents)
	{
		m_contents = make_shared< Contents >(contents);
	}
	
	template < typename MortonPrecision, typename Float, typename Vec3, typename Contents>
	inline shared_ptr< Contents > InnerNode<MortonPrecision, Float, Vec3, Contents>::
		getContents() const
	{
		return m_contents;
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Contents >
	ostream& operator<<( ostream& out, const InnerNode< MortonPrecision, Float, Vec3, Contents >& node )
	{
		out << "Inner Node: " << endl << *node.getContents();
		return out;
	}
}
	
#endif