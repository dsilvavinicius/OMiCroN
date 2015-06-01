#ifndef FRONT_H
#define FRONT_H

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/member.hpp>

#include "MortonCode.h"

using namespace boost::multi_index;

namespace model
{
	/** This struct has all types used to represent the front as a set with an ordered index and a hashed index. */
	template< typename MortonCode >
	struct FrontTypes
	{
		struct sequential {};
		struct morton {};
		
		/** Front entry. Has all the necessary info to query a node on the octree and to find the related data in the point array
		 * used for rendering. */;
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
		
		/** The container has the actual front entries. */
		using Front = multi_index_container<
			Entry,
			indexed_by<
				sequenced< tag< sequential > >,
				hashed_unique<
					tag< morton >, member< Entry, MortonCode, &Entry::m_code > >
			>
		>;
		
		using FrontBySequence = typename Front::template index< sequential >::type;
		using FrontByMorton = typename Front::template index< morton >::type;
	};
	
	using ShallowFront = FrontTypes< unsigned int >::Front;
	using DeepFront = FrontTypes< unsigned long >::Front;
}

#endif