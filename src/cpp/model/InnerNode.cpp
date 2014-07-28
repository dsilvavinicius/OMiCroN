#include "InnerNode.h"

namespace model
{
	template <typename MortonPrecision, typename Contents>
	bool InnerNode<MortonPrecision, Contents>::isLeaf()
	{
		return false;
	}
	
	template <typename MortonPrecision, typename Contents>
	void InnerNode<MortonPrecision, Contents>::setContents(Contents contents)
	{
		m_contents = contents;
	}
	
	template <typename MortonPrecision, typename Contents>
	Contents InnerNode<MortonPrecision, Contents>::getContents()
	{
		return m_contents;
	}
}