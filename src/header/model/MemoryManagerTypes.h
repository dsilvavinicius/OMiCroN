#ifndef MEMORY_MANAGER_TYPES
#define MEMORY_MANAGER_TYPES
#include "BitMapMemoryManager.h"
#include "MortonCode.h"
#include "InnerNode.h"
#include "LeafNode.h"
#include "Ken12MemoryManager.h"

namespace model
{
	// Setup presets for memory managers.
	
	// ===================
	// BitMapMemoryManager
	// ===================
	
	/** Shallow, Point, Vector inner. */
	using SPV_BitMapMemoryManager = BitMapMemoryManager<	ShallowMortonCode, Point, InnerNode< PointVector >,
															LeafNode< PointVector > >;

	/** Shallow, Point, Point inner. */
	using SPP_BitMapMemoryManager = BitMapMemoryManager<	ShallowMortonCode, Point, InnerNode< PointPtr >,
															LeafNode< PointVector > >;

	/** Shallow, Point, Index inner. */
	using SPI_BitMapMemoryManager = BitMapMemoryManager<	ShallowMortonCode, Point, InnerNode< IndexVector >,
															LeafNode< IndexVector > >;
															
	/** Shallow, ExtendedPoint, Vector inner. */
	using SEV_BitMapMemoryManager = BitMapMemoryManager< 	ShallowMortonCode, ExtendedPoint,
															InnerNode< ExtendedPointVector >,
															LeafNode< ExtendedPointVector > >;
	
	/** Shallow, ExtendedPoint, Point inner. */
	using SEP_BitMapMemoryManager = BitMapMemoryManager<	ShallowMortonCode, ExtendedPoint,
															InnerNode< ExtendedPointPtr >,
															LeafNode< PointVector > >;
	
	/** Shallow, ExtendedPoint, Index inner. */
	using SEI_BitMapMemoryManager = BitMapMemoryManager<	ShallowMortonCode, ExtendedPoint, InnerNode< IndexVector >,
															LeafNode< IndexVector > >;
	
	/** Medium, Point, Vector inner. */
	using MPV_BitMapMemoryManager = BitMapMemoryManager<	MediumMortonCode, Point, InnerNode< PointVector >,
															LeafNode< PointVector > >;

	/** Medium, Point, Point inner. */
	using MPP_BitMapMemoryManager = BitMapMemoryManager<	MediumMortonCode, Point, InnerNode< PointPtr >,
															LeafNode< PointVector > >;
															
	/** Medium, Point, Index inner. */
	using MPI_BitMapMemoryManager = BitMapMemoryManager<	MediumMortonCode, Point, InnerNode< IndexVector >,
															LeafNode< IndexVector > >;

	/** Medium, ExtendedPoint, Vector inner. */
	using MEV_BitMapMemoryManager = BitMapMemoryManager<	MediumMortonCode, ExtendedPoint,
															InnerNode< ExtendedPointVector >,
															LeafNode< ExtendedPointVector > >;
	
	/** Medium, ExtendedPoint, Point inner. */
	using MEP_BitMapMemoryManager = BitMapMemoryManager<	MediumMortonCode, ExtendedPoint,
															InnerNode< ExtendedPointPtr >,
															LeafNode< PointVector > >;

	/** Medium, ExtendedPoint, Index inner. */
	using MEI_BitMapMemoryManager = BitMapMemoryManager<	MediumMortonCode, ExtendedPoint, InnerNode< IndexVector >,
															LeafNode< IndexVector > >;

	// ===================
	// Ken12MemoryManager
	// ===================
	
	/** Shallow, Point, Vector inner. */
	using SPV_Ken12MemoryManager = Ken12MemoryManager<	ShallowMortonCode, Point, InnerNode< PointVector >,
														LeafNode< PointVector > >;

	/** Shallow, Point, Point inner. */
	using SPP_Ken12MemoryManager = Ken12MemoryManager<	ShallowMortonCode, Point, InnerNode< PointPtr >,
														LeafNode< PointVector > >;

	/** Shallow, Point, Index inner. */
	using SPI_Ken12MemoryManager = Ken12MemoryManager<	ShallowMortonCode, Point, InnerNode< IndexVector >,
														LeafNode< IndexVector > >;
															
	/** Shallow, ExtendedPoint, Vector inner. */
	using SEV_Ken12MemoryManager = Ken12MemoryManager< 	ShallowMortonCode, ExtendedPoint,
														InnerNode< ExtendedPointVector >,
														LeafNode< ExtendedPointVector > >;
	
	/** Shallow, ExtendedPoint, Point inner. */
	using SEP_Ken12MemoryManager = Ken12MemoryManager<	ShallowMortonCode, ExtendedPoint,
														InnerNode< ExtendedPointPtr >,
														LeafNode< PointVector > >;
	
	/** Shallow, ExtendedPoint, Index inner. */
	using SEI_Ken12MemoryManager = Ken12MemoryManager<	ShallowMortonCode, ExtendedPoint,
														InnerNode< IndexVector >,
														LeafNode< IndexVector > >;
	
	/** Medium, Point, Vector inner. */
	using MPV_Ken12MemoryManager = Ken12MemoryManager<	MediumMortonCode, Point, InnerNode< PointVector >,
														LeafNode< PointVector > >;

	/** Medium, Point, Point inner. */
	using MPP_Ken12MemoryManager = Ken12MemoryManager<	MediumMortonCode, Point, InnerNode< PointPtr >,
														LeafNode< PointVector > >;
															
	/** Medium, Point, Index inner. */
	using MPI_Ken12MemoryManager = Ken12MemoryManager<	MediumMortonCode, Point, InnerNode< IndexVector >,
														LeafNode< IndexVector > >;

	/** Medium, ExtendedPoint, Vector inner. */
	using MEV_Ken12MemoryManager = Ken12MemoryManager<	MediumMortonCode, ExtendedPoint,
														InnerNode< ExtendedPointVector >,
														LeafNode< ExtendedPointVector > >;
	
	/** Medium, ExtendedPoint, Point inner. */
	using MEP_Ken12MemoryManager = Ken12MemoryManager<	MediumMortonCode, ExtendedPoint,
														InnerNode< ExtendedPointPtr >,
														LeafNode< PointVector > >;

	/** Medium, ExtendedPoint, Index inner. */
	using MEI_Ken12MemoryManager = Ken12MemoryManager<	MediumMortonCode, ExtendedPoint,
														InnerNode< IndexVector >,
														LeafNode< IndexVector > >;
}

#endif