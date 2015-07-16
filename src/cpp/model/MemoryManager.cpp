#include "MemoryManager.h"
#include <MortonCode.h>
#include <LeafNode.h>

namespace model
{
	uint MemoryManager::SHALLOW_MORTON_SIZE = sizeof( ShallowMortonCode );
	uint MemoryManager::MEDIUM_MORTON_SIZE = sizeof( MediumMortonCode );
	uint MemoryManager::POINT_SIZE = sizeof( Point );
	uint MemoryManager::EXTENDED_POINT_SIZE = sizeof( ExtendedPoint );
	uint MemoryManager::NODE_SIZE = sizeof( ShallowLeafNode< PointVector > );
	
	MemoryManager MemoryManager::m_instance;
}