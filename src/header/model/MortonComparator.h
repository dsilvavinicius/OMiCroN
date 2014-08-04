#ifndef MORTON_COMPARATOR_H
#define MORTON_COMPARATOR_H

#include "MortonCode.h"

namespace model
{

	/** Compares MortonCode. */
	template <typename MortonPrecision>
	class MortonComparator
	{
	public:
		bool operator()(const MortonCodePtr< MortonPrecision >& a, const MortonCodePtr< MortonPrecision >& b) const;
	};

	template <typename MortonPrecision>
	bool MortonComparator< MortonPrecision >::operator()(const MortonCodePtr< MortonPrecision >& a,
														 const MortonCodePtr< MortonPrecision >& b) const
	{
		return a->getBits() < b->getBits();
	}

	//============
	// Type sugar.
	//============

	using ShallowMortonComparator = MortonComparator<unsigned int>;
	using MediumMortonComparator = MortonComparator<unsigned long>;
	using DeepMortonComparator = MortonComparator<unsigned long long>;
}

#endif