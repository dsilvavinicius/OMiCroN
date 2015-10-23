#ifndef MEMORY_MANAGER_TYPES
#define MEMORY_MANAGER_TYPES
#include "BitMapMemoryManager.h"
#include "MortonCode.h"
#include "OctreeNode.h"
#include "Ken12MemoryManager.h"
#include "TLSFManager.h"

namespace model
{
	// ==============================================================================================================
	/** DEFAULT MANAGER. Will be used for all manager-type-agnostic tests and the system itself.
	 * If another manager is desired for testing purposes, it must be explicitily initialized by the test itself.  */
	// ==============================================================================================================
	
	template< typename Morton, typename Point, typename Node >
	using DefaultManager = TLSFManager< Morton, Point, Node >;

	/** Shallow, Point, Vector node. */
	using SPV_DefaultManager = DefaultManager< ShallowMortonCode, Point, OctreeNode< PointVector > >;

	/** Shallow, Point, Index node. */
	using SPI_DefaultManager = DefaultManager< ShallowMortonCode, Point, OctreeNode< IndexVector > >;
															
	/** Shallow, ExtendedPoint, Vector node. */
	using SEV_DefaultManager = DefaultManager< ShallowMortonCode, ExtendedPoint, OctreeNode< ExtendedPointVector > >;
	
	/** Shallow, ExtendedPoint, Index node. */
	using SEI_DefaultManager = DefaultManager< ShallowMortonCode, ExtendedPoint, OctreeNode< IndexVector > >;
	
	/** Medium, Point, Vector node. */
	using MPV_DefaultManager = DefaultManager< MediumMortonCode, Point, OctreeNode< PointVector > >;
															
	/** Medium, Point, Index node. */
	using MPI_DefaultManager = DefaultManager< MediumMortonCode, Point, OctreeNode< IndexVector > >;

	/** Medium, ExtendedPoint, Vector node. */
	using MEV_DefaultManager = DefaultManager< MediumMortonCode, ExtendedPoint, OctreeNode< ExtendedPointVector > >;

	/** Medium, ExtendedPoint, Index node. */
	using MEI_DefaultManager = DefaultManager< MediumMortonCode, ExtendedPoint, OctreeNode< IndexVector > >;
}

#endif