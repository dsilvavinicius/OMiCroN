#ifndef MORTON_INTERVAL_H
#define MORTON_INTERVAL_H

#include "MortonCode.h"

namespace model
{
	template< typename MortonCode >
	using MortonInterval = pair< shared_ptr< MortonCode >, shared_ptr< MortonCode > >;
	
	template< typename MortonInterval >
	struct MortonIntervalComparator
	{
		bool operator()( const MortonInterval& a, const MortonInterval& b ) const
		{
			return *a.first == *b.first && *a.second == *b.second;
		}
	};
	
	template< typename MortonCode >
	ostream& operator<<( ostream& out, const MortonInterval< MortonCode >& interval )
	{
		out << "(" <<  endl << interval.first->getPathToRoot( true ) << ","
			<< interval.second->getPathToRoot( true ) << ")" << endl;
			
		return out;
	}
	
	//=================
	// Type sugar.
	//=================
	
	using ShallowMortonInterval = MortonInterval< ShallowMortonCode >;
	using MediumMortonInterval = MortonInterval< MediumMortonCode >;
	
	using ShallowMortonIntervalComparator = MortonIntervalComparator< ShallowMortonInterval >;
	using MediumMortonIntervalComparator = MortonIntervalComparator< MediumMortonInterval >;
}

namespace std
{
	template <>
	struct hash< model::ShallowMortonInterval >
	{
		size_t operator()( const model::ShallowMortonInterval& k ) const
		{
			return( ( hash< model::ShallowMortonCode >()( *k.first ) ^
					( hash< model::ShallowMortonCode >()( *k.second ) << 1 ) ) >> 1);
		}
	};
	
	template <>
	struct hash< typename model::MediumMortonInterval >
	{
		size_t operator()( const model::MediumMortonInterval& k ) const
		{
			return( ( hash< model::MediumMortonCode >()( *k.first ) ^
					( hash< model::MediumMortonCode >()( *k.second ) << 1 ) ) >> 1);
		}
	};
}

#endif