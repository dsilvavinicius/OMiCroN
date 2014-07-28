#include "LeafNode.h"

namespace model
{
	template <typename MortonPrecision, typename Contents>
	bool LeafNode<MortonPrecision, Contents>::isLeaf()
	{
		return true;
	}
	
	template <typename MortonPrecision, typename Contents>
	void LeafNode<MortonPrecision, Contents>::setContents(Contents contents)
	{
		m_contents = contents;
	}
	
	template <typename MortonPrecision, typename Contents>
	Contents LeafNode<MortonPrecision, Contents>::getContents()
	{
		return m_contents;
	}
}