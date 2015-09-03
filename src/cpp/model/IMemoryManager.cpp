#include "IMemoryManager.h"

namespace model
{
	unique_ptr< IMemoryManager> SingletonMemoryManager::m_instance = nullptr;
}