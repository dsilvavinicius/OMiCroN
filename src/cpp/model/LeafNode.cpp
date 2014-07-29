#include "LeafNode.h"

namespace model
{
	template < typename MortonPrecision, typename Float, typename Vec3,
		typename Contents >
	bool LeafNode<MortonPrecision, Float, Vec3, Contents>::isLeaf()
	{
		return true;
	}
	
	template < typename MortonPrecision, typename Float, typename Vec3,
		typename Contents >
	void LeafNode<MortonPrecision, Float, Vec3, Contents>::setContents(
		const Contents& contents)
	{
		m_contents = make_shared< Contents>(contents);
	}
	
	template < typename MortonPrecision, typename Float, typename Vec3,
		typename Contents >
	shared_ptr< Contents > LeafNode<MortonPrecision, Float, Vec3, Contents>::
		getContents()
	{
		return m_contents;
	}
}