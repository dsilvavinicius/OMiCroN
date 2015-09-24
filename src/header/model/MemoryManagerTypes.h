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
	using SPV_BitMapMemoryManager = BitMapMemoryManager<	ShallowMortonCode, Point, ShallowInnerNode< PointVector >,
															ShallowLeafNode< PointVector > >;

	/** Shallow, Point, Point inner. */
	using SPP_BitMapMemoryManager = BitMapMemoryManager<	ShallowMortonCode, Point, ShallowInnerNode< PointPtr >,
															ShallowLeafNode< PointVector > >;

	/** Shallow, Point, Index inner. */
	using SPI_BitMapMemoryManager = BitMapMemoryManager<	ShallowMortonCode, Point, ShallowInnerNode< vector< uint > >,
															ShallowLeafNode< vector< uint > > >;
															
	/** Shallow, ExtendedPoint, Vector inner. */
	using SEV_BitMapMemoryManager = BitMapMemoryManager< 	ShallowMortonCode, ExtendedPoint,
															ShallowInnerNode< ExtendedPointVector >,
															ShallowLeafNode< ExtendedPointVector > >;
	
	/** Shallow, ExtendedPoint, Point inner. */
	using SEP_BitMapMemoryManager = BitMapMemoryManager<	ShallowMortonCode, ExtendedPoint,
															ShallowInnerNode< ExtendedPointPtr >,
															ShallowLeafNode< PointVector > >;
	
	/** Shallow, ExtendedPoint, Index inner. */
	using SEI_BitMapMemoryManager = BitMapMemoryManager<	ShallowMortonCode, ExtendedPoint,
															ShallowInnerNode< vector< uint > >,
															ShallowLeafNode< vector< uint > > >;
	
	/** Medium, Point, Vector inner. */
	using MPV_BitMapMemoryManager = BitMapMemoryManager<	MediumMortonCode, Point, MediumInnerNode< PointVector >,
															MediumLeafNode< PointVector > >;

	/** Medium, Point, Point inner. */
	using MPP_BitMapMemoryManager = BitMapMemoryManager<	MediumMortonCode, Point, MediumInnerNode< PointPtr >,
															MediumLeafNode< PointVector > >;
															
	/** Medium, Point, Index inner. */
	using MPI_BitMapMemoryManager = BitMapMemoryManager<	MediumMortonCode, Point, MediumInnerNode< vector< uint > >,
															MediumLeafNode< vector< uint > > >;

	/** Medium, ExtendedPoint, Vector inner. */
	using MEV_BitMapMemoryManager = BitMapMemoryManager<	MediumMortonCode, ExtendedPoint,
															MediumInnerNode< ExtendedPointVector >,
															MediumLeafNode< ExtendedPointVector > >;
	
	/** Medium, ExtendedPoint, Point inner. */
	using MEP_BitMapMemoryManager = BitMapMemoryManager<	MediumMortonCode, ExtendedPoint,
															MediumInnerNode< ExtendedPointPtr >,
															MediumLeafNode< PointVector > >;

	/** Medium, ExtendedPoint, Index inner. */
	using MEI_BitMapMemoryManager = BitMapMemoryManager<	MediumMortonCode, ExtendedPoint,
															MediumInnerNode< vector< uint > >,
															MediumLeafNode< vector< uint > > >;

	// ===================
	// Ken12MemoryManager
	// ===================
	
	/** Shallow, Point, Vector inner. */
	using SPV_Ken12MemoryManager = Ken12MemoryManager<	ShallowMortonCode, Point, ShallowInnerNode< PointVector >,
														ShallowLeafNode< PointVector > >;

	/** Shallow, Point, Point inner. */
	using SPP_Ken12MemoryManager = Ken12MemoryManager<	ShallowMortonCode, Point, ShallowInnerNode< PointPtr >,
															ShallowLeafNode< PointVector > >;

	/** Shallow, Point, Index inner. */
	using SPI_Ken12MemoryManager = Ken12MemoryManager<	ShallowMortonCode, Point, ShallowInnerNode< vector< uint > >,
															ShallowLeafNode< vector< uint > > >;
															
	/** Shallow, ExtendedPoint, Vector inner. */
	using SEV_Ken12MemoryManager = Ken12MemoryManager< 	ShallowMortonCode, ExtendedPoint,
															ShallowInnerNode< ExtendedPointVector >,
															ShallowLeafNode< ExtendedPointVector > >;
	
	/** Shallow, ExtendedPoint, Point inner. */
	using SEP_Ken12MemoryManager = Ken12MemoryManager<	ShallowMortonCode, ExtendedPoint,
															ShallowInnerNode< ExtendedPointPtr >,
															ShallowLeafNode< PointVector > >;
	
	/** Shallow, ExtendedPoint, Index inner. */
	using SEI_Ken12MemoryManager = Ken12MemoryManager<	ShallowMortonCode, ExtendedPoint,
															ShallowInnerNode< vector< uint > >,
															ShallowLeafNode< vector< uint > > >;
	
	/** Medium, Point, Vector inner. */
	using MPV_Ken12MemoryManager = Ken12MemoryManager<	MediumMortonCode, Point, MediumInnerNode< PointVector >,
															MediumLeafNode< PointVector > >;

	/** Medium, Point, Point inner. */
	using MPP_Ken12MemoryManager = Ken12MemoryManager<	MediumMortonCode, Point, MediumInnerNode< PointPtr >,
															MediumLeafNode< PointVector > >;
															
	/** Medium, Point, Index inner. */
	using MPI_Ken12MemoryManager = Ken12MemoryManager<	MediumMortonCode, Point, MediumInnerNode< vector< uint > >,
															MediumLeafNode< vector< uint > > >;

	/** Medium, ExtendedPoint, Vector inner. */
	using MEV_Ken12MemoryManager = Ken12MemoryManager<	MediumMortonCode, ExtendedPoint,
															MediumInnerNode< ExtendedPointVector >,
															MediumLeafNode< ExtendedPointVector > >;
	
	/** Medium, ExtendedPoint, Point inner. */
	using MEP_Ken12MemoryManager = Ken12MemoryManager<	MediumMortonCode, ExtendedPoint,
															MediumInnerNode< ExtendedPointPtr >,
															MediumLeafNode< PointVector > >;

	/** Medium, ExtendedPoint, Index inner. */
	using MEI_Ken12MemoryManager = Ken12MemoryManager<	MediumMortonCode, ExtendedPoint,
															MediumInnerNode< vector< uint > >,
															MediumLeafNode< vector< uint > > >;
}

#endif