#ifndef FRONT_H
#define FRONT_H

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>
#include "MortonCode.h"

using boost::multi_index_container;
using namespace ::boost;
using namespace boost::multi_index;

namespace model
{
	/** Bidirectional map with an ordered index and a hashed index. */
	template
	struct Front
	{
		struct sequential {};
		struct hashed {};
		
		struct Entry
		{
			Entry( const unsigned int& offset, const int& nPoints, const MortonCode& code ):
			m_offset( offset ),
			m_nPoints( nPoints ),
			m_code( code )
			{}

			/** The offset, relative to the GPU array pointer, where the points owned by the node represented
			 * by this entry are. */
			unsigned int m_offset;
			
			/** Number of points that the node represented by this entry contains. */
			int m_nPoints;
			
			/** Identifier of the octree node represented by this entry. */
			MortonCode m_code;
		};
		
		using Container = multi_index_container<
			ValueType,
			indexed_by<
				sequenced< tag< sequential > >,
				hashed_unique<
					tag< hashed >, member< Entry, MortonCode, &Entry::m_code > >
			>
		>;
	};
	
	using ShallowFront = Front< unsigned int >;
	using DeepFront = Front< unsigned long >;
}

#endif