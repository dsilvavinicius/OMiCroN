#ifndef OCTREE_MAP_TYPES_H
#define OCTREE_MAP_TYPES_H

#include <memory>
#include <map>
#include "MortonCode.h"
#include "MortonComparator.h"
#include "OctreeNode.h"

namespace model
{
	// =========
	// OctreeMap
	// =========
	
	/** Internal map type used to actualy store the octree. */
	template< typename Morton, typename Node, typename Allocator >
	using OctreeMapBase = map< shared_ptr< Morton >, shared_ptr< Node >, MortonComparator< Morton >, Allocator >;

	/** OctreeMap with default allocator. */
	template< typename Morton, typename Node >
	using DefaultOctreeMap = OctreeMapBase< Morton, Node,
											std::allocator< pair< const shared_ptr< Morton >, shared_ptr< Node > > > >;
	
	/** OctreeMap with ManagedAllocator. */
	template< typename Morton, typename Node >
	using OctreeMap = OctreeMapBase< Morton, Node,
									 ManagedAllocator< pair< const shared_ptr< Morton >, shared_ptr< Node > > > >;
	
	/** Smart pointer for the internal octree map. */
	template < typename Morton, typename Node >
	using OctreeMapPtr = shared_ptr< OctreeMap< Morton, Node > >;

	/** 32-bit morton code octree map. */
	template< typename Node >
	using ShallowOctreeMap = OctreeMap< ShallowMortonCode, Node >;

	/** 32-bit morton code octree map pointer. */
	template< typename Node >
	using ShallowOctreeMapPtr = shared_ptr< ShallowOctreeMap< Node > >;

	/** 64-bit morton code octree map. */
	template< typename Node >
	using MediumOctreeMap = OctreeMap< MediumMortonCode, Node >; 

	/** 64-bit morton code octree map pointer. */
	template< typename Node >
	using MediumOctreeMapPtr = shared_ptr< MediumOctreeMap< Node > >;
}

#endif