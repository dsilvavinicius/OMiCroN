#include "MemoryManager.h"
#include <MortonCode.h>
#include <LeafNode.h>

namespace model
{
	uint MemoryManager::SIZES[] = { sizeof( ShallowMortonCode ), sizeof( MediumMortonCode ), sizeof( Point ),
									sizeof( ExtendedPoint ), sizeof( ShallowLeafNode< PointVector > ) };
	
	MemoryManager MemoryManager::m_instance;
}