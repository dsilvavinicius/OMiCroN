#ifndef OCTREE_PARAMETER_TYPES
#define OCTREE_PARAMETER_TYPES

#include "MortonCode.h"
#include "ExtendedPoint.h"
#include "OctreeNode.h"
#include "O1OctreeNode.h"

namespace model
{
	template< typename M, typename P, typename N, typename H >
	struct OctreeParameters
	{
		using Morton = M;
		using Point = P;
		using Node = N;
		using Hierarchy = H;
	};
	
	template< typename Morton, typename Point, typename Node, typename Hierarchy >
	using OctreeParameters = struct OctreeParameters< Morton, Point, Node, Hierarchy >;
}

#endif