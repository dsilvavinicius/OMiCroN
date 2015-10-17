#ifndef MEMORY_MANAGER_TYPES
#define MEMORY_MANAGER_TYPES
#include "BitMapMemoryManager.h"
#include "MortonCode.h"
#include "InnerNode.h"
#include "LeafNode.h"
#include "Ken12MemoryManager.h"
#include "TLSFManager.h"

namespace model
{
	// ==============================================================================================================
	/** DEFAULT MANAGER. Will be used for all manager-type-agnostic tests and the system itself.
	 * If another manager is desired for testing purposes, it must be explicitily initialized by the test itself.  */
	// ==============================================================================================================
	
	template< typename Morton, typename Point, typename Inner, typename Leaf >
	using DefaultManager = TLSFManager< Morton, Point, Inner, Leaf >;

	/** Shallow, Point, Vector inner. */
	using SPV_DefaultManager = DefaultManager<	ShallowMortonCode, Point, InnerNode< PointVector >,
															LeafNode< PointVector > >;

	/** Shallow, Point, Point inner. */
	using SPP_DefaultManager = DefaultManager<	ShallowMortonCode, Point, InnerNode< PointPtr >,
															LeafNode< PointVector > >;

	/** Shallow, Point, Index inner. */
	using SPI_DefaultManager = DefaultManager<	ShallowMortonCode, Point, InnerNode< IndexVector >,
															LeafNode< IndexVector > >;
															
	/** Shallow, ExtendedPoint, Vector inner. */
	using SEV_DefaultManager = DefaultManager< 	ShallowMortonCode, ExtendedPoint,
															InnerNode< ExtendedPointVector >,
															LeafNode< ExtendedPointVector > >;
	
	/** Shallow, ExtendedPoint, Point inner. */
	using SEP_DefaultManager = DefaultManager<	ShallowMortonCode, ExtendedPoint,
															InnerNode< ExtendedPointPtr >,
															LeafNode< PointVector > >;
	
	/** Shallow, ExtendedPoint, Index inner. */
	using SEI_DefaultManager = DefaultManager<	ShallowMortonCode, ExtendedPoint, InnerNode< IndexVector >,
															LeafNode< IndexVector > >;
	
	/** Medium, Point, Vector inner. */
	using MPV_DefaultManager = DefaultManager<	MediumMortonCode, Point, InnerNode< PointVector >,
															LeafNode< PointVector > >;

	/** Medium, Point, Point inner. */
	using MPP_DefaultManager = DefaultManager<	MediumMortonCode, Point, InnerNode< PointPtr >,
															LeafNode< PointVector > >;
															
	/** Medium, Point, Index inner. */
	using MPI_DefaultManager = DefaultManager<	MediumMortonCode, Point, InnerNode< IndexVector >,
															LeafNode< IndexVector > >;

	/** Medium, ExtendedPoint, Vector inner. */
	using MEV_DefaultManager = DefaultManager<	MediumMortonCode, ExtendedPoint,
															InnerNode< ExtendedPointVector >,
															LeafNode< ExtendedPointVector > >;
	
	/** Medium, ExtendedPoint, Point inner. */
	using MEP_DefaultManager = DefaultManager<	MediumMortonCode, ExtendedPoint,
															InnerNode< ExtendedPointPtr >,
															LeafNode< PointVector > >;

	/** Medium, ExtendedPoint, Index inner. */
	using MEI_DefaultManager = DefaultManager<	MediumMortonCode, ExtendedPoint, InnerNode< IndexVector >,
															LeafNode< IndexVector > >;
	
// 	// Setup presets for memory managers.
// 	
// 	// ===================
// 	// BitMapMemoryManager
// 	// ===================
// 	
// 	/** Shallow, Point, Vector inner. */
// 	using SPV_DefaultManager = BitMapMemoryManager<	ShallowMortonCode, Point, InnerNode< PointVector >,
// 															LeafNode< PointVector > >;
// 
// 	/** Shallow, Point, Point inner. */
// 	using SPP_DefaultManager = BitMapMemoryManager<	ShallowMortonCode, Point, InnerNode< PointPtr >,
// 															LeafNode< PointVector > >;
// 
// 	/** Shallow, Point, Index inner. */
// 	using SPI_DefaultManager = BitMapMemoryManager<	ShallowMortonCode, Point, InnerNode< IndexVector >,
// 															LeafNode< IndexVector > >;
// 															
// 	/** Shallow, ExtendedPoint, Vector inner. */
// 	using SEV_DefaultManager = BitMapMemoryManager< 	ShallowMortonCode, ExtendedPoint,
// 															InnerNode< ExtendedPointVector >,
// 															LeafNode< ExtendedPointVector > >;
// 	
// 	/** Shallow, ExtendedPoint, Point inner. */
// 	using SEP_DefaultManager = BitMapMemoryManager<	ShallowMortonCode, ExtendedPoint,
// 															InnerNode< ExtendedPointPtr >,
// 															LeafNode< PointVector > >;
// 	
// 	/** Shallow, ExtendedPoint, Index inner. */
// 	using SEI_DefaultManager = BitMapMemoryManager<	ShallowMortonCode, ExtendedPoint, InnerNode< IndexVector >,
// 															LeafNode< IndexVector > >;
// 	
// 	/** Medium, Point, Vector inner. */
// 	using MPV_DefaultManager = BitMapMemoryManager<	MediumMortonCode, Point, InnerNode< PointVector >,
// 															LeafNode< PointVector > >;
// 
// 	/** Medium, Point, Point inner. */
// 	using MPP_DefaultManager = BitMapMemoryManager<	MediumMortonCode, Point, InnerNode< PointPtr >,
// 															LeafNode< PointVector > >;
// 															
// 	/** Medium, Point, Index inner. */
// 	using MPI_DefaultManager = BitMapMemoryManager<	MediumMortonCode, Point, InnerNode< IndexVector >,
// 															LeafNode< IndexVector > >;
// 
// 	/** Medium, ExtendedPoint, Vector inner. */
// 	using MEV_DefaultManager = BitMapMemoryManager<	MediumMortonCode, ExtendedPoint,
// 															InnerNode< ExtendedPointVector >,
// 															LeafNode< ExtendedPointVector > >;
// 	
// 	/** Medium, ExtendedPoint, Point inner. */
// 	using MEP_DefaultManager = BitMapMemoryManager<	MediumMortonCode, ExtendedPoint,
// 															InnerNode< ExtendedPointPtr >,
// 															LeafNode< PointVector > >;
// 
// 	/** Medium, ExtendedPoint, Index inner. */
// 	using MEI_DefaultManager = BitMapMemoryManager<	MediumMortonCode, ExtendedPoint, InnerNode< IndexVector >,
// 															LeafNode< IndexVector > >;
// 
// 	// ===================
// 	// Ken12MemoryManager
// 	// ===================
// 	
// 	/** Shallow, Point, Vector inner. */
// 	using SPV_Ken12MemoryManager = Ken12MemoryManager<	ShallowMortonCode, Point, InnerNode< PointVector >,
// 														LeafNode< PointVector > >;
// 
// 	/** Shallow, Point, Point inner. */
// 	using SPP_Ken12MemoryManager = Ken12MemoryManager<	ShallowMortonCode, Point, InnerNode< PointPtr >,
// 														LeafNode< PointVector > >;
// 
// 	/** Shallow, Point, Index inner. */
// 	using SPI_Ken12MemoryManager = Ken12MemoryManager<	ShallowMortonCode, Point, InnerNode< IndexVector >,
// 														LeafNode< IndexVector > >;
// 															
// 	/** Shallow, ExtendedPoint, Vector inner. */
// 	using SEV_Ken12MemoryManager = Ken12MemoryManager< 	ShallowMortonCode, ExtendedPoint,
// 														InnerNode< ExtendedPointVector >,
// 														LeafNode< ExtendedPointVector > >;
// 	
// 	/** Shallow, ExtendedPoint, Point inner. */
// 	using SEP_Ken12MemoryManager = Ken12MemoryManager<	ShallowMortonCode, ExtendedPoint,
// 														InnerNode< ExtendedPointPtr >,
// 														LeafNode< PointVector > >;
// 	
// 	/** Shallow, ExtendedPoint, Index inner. */
// 	using SEI_Ken12MemoryManager = Ken12MemoryManager<	ShallowMortonCode, ExtendedPoint,
// 														InnerNode< IndexVector >,
// 														LeafNode< IndexVector > >;
// 	
// 	/** Medium, Point, Vector inner. */
// 	using MPV_Ken12MemoryManager = Ken12MemoryManager<	MediumMortonCode, Point, InnerNode< PointVector >,
// 														LeafNode< PointVector > >;
// 
// 	/** Medium, Point, Point inner. */
// 	using MPP_Ken12MemoryManager = Ken12MemoryManager<	MediumMortonCode, Point, InnerNode< PointPtr >,
// 														LeafNode< PointVector > >;
// 															
// 	/** Medium, Point, Index inner. */
// 	using MPI_Ken12MemoryManager = Ken12MemoryManager<	MediumMortonCode, Point, InnerNode< IndexVector >,
// 														LeafNode< IndexVector > >;
// 
// 	/** Medium, ExtendedPoint, Vector inner. */
// 	using MEV_Ken12MemoryManager = Ken12MemoryManager<	MediumMortonCode, ExtendedPoint,
// 														InnerNode< ExtendedPointVector >,
// 														LeafNode< ExtendedPointVector > >;
// 	
// 	/** Medium, ExtendedPoint, Point inner. */
// 	using MEP_Ken12MemoryManager = Ken12MemoryManager<	MediumMortonCode, ExtendedPoint,
// 														InnerNode< ExtendedPointPtr >,
// 														LeafNode< PointVector > >;
// 
// 	/** Medium, ExtendedPoint, Index inner. */
// 	using MEI_Ken12MemoryManager = Ken12MemoryManager<	MediumMortonCode, ExtendedPoint,
// 														InnerNode< IndexVector >,
// 														LeafNode< IndexVector > >;
}

#endif