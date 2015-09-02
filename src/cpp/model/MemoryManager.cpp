#include "MemoryManager.h"
#include <MortonCode.h>
#include <LeafNode.h>

namespace model
{
	uint MemoryManager::M_SIZES[] = { 4, 8, 24, 32, 36 };
	
	MemoryManager MemoryManager::m_instance;
}