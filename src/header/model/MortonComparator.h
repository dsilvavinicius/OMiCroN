#ifndef MORTON_COMPARATOR_H
#define MORTON_COMPARATOR_H

#include "MortonCode.h"

namespace model
{

	/** Compares MortonCode. */
	template< typename MortonCode >
	class MortonComparator
	{
		using MortonCodePtr = shared_ptr< MortonCode >;
	public:
		bool operator()( const MortonCodePtr& a, const MortonCodePtr& b ) const;
	};

	template< typename MortonCode >
	bool MortonComparator< MortonCode >::operator()( const MortonCodePtr& a, const MortonCodePtr& b ) const
	{
		return a->getBits() < b->getBits();
	}

	//============
	// Type sugar.
	//============

	using ShallowMortonComparator = MortonComparator< ShallowMortonCode >;
	using MediumMortonComparator = MortonComparator< MediumMortonCode >;
	using DeepMortonComparator = MortonComparator< DeepMortonCode >;
}

#endif