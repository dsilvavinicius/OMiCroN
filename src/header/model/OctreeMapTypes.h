#ifndef OCTREE_MAP_TYPES_H
#define OCTREE_MAP_TYPES_H

#include <memory>
#include <map>
#include "MortonCode.h"
#include "MortonComparator.h"
#include "OctreeNode.h"

namespace model
{	
	/** Internal map type used to actualy store the octree. */
	template< typename MortonCode, typename Allocator >
	using OctreeMapBase = map< shared_ptr< MortonCode >, OctreeNodePtr, MortonComparator< MortonCode >, Allocator >;

	/** OctreeMap with default allocator. */
	template< typename MortonCode >
	using DefaultOctreeMap = OctreeMapBase< MortonCode, std::allocator< pair< const shared_ptr< MortonCode >, OctreeNodePtr > > >;
	
	/** OctreeMap with BitMapAllocator. */
	template< typename MortonCode >
	using OctreeMap = OctreeMapBase< MortonCode, BitMapAllocator< pair< const shared_ptr< MortonCode >, OctreeNodePtr > > >;
	
							
	/** Smart pointer for the internal octree map. */
	template < typename MortonCode >
	using OctreeMapPtr = shared_ptr< OctreeMap< MortonCode > >;

	/** 32-bit morton code octree map. */
	using ShallowOctreeMap = OctreeMap< ShallowMortonCode >;

	/** 32-bit morton code octree map pointer. */
	using ShallowOctreeMapPtr = shared_ptr< ShallowOctreeMap >;

	/** 64-bit morton code octree map. */
	using MediumOctreeMap = OctreeMap< MediumMortonCode >; 

	/** 64-bit morton code octree map pointer. */
	using MediumOctreeMapPtr = shared_ptr< MediumOctreeMap >;
}

#endif