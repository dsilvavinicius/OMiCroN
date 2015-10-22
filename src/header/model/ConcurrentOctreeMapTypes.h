#ifndef CONCURRENT_OCTREE_MAP_TYPES
#define CONCURRENT_OCTREE_MAP_TYPES

#include <tbb/concurrent_hash_map.h>
#include "O1OctreeNode.h"

using namespace tbb;

namespace model
{
	// ===================
	// ConcurrentOctreeMap
	// ===================
	
	template< typename Morton, typename Node, typename Allocator >
	using ConcurrentOctreeMapBase = concurrent_hash_map< Morton, Node, tbb_hash_compare< Morton >, Allocator >;
	
	template< typename Morton, typename Node >
	using DefaultConcurrentOctreeMap = ConcurrentOctreeMapBase< Morton, Node,
		std::allocator< pair< shared_ptr< Morton >, shared_ptr< Node > > > >;
		
	template< typename Morton, typename Node >
	using ConcurrentOctreeMap = ConcurrentOctreeMapBase< Morton, Node,
		ManagedAllocator< pair< shared_ptr< Morton >, shared_ptr< Node > > > >;
}

#endif