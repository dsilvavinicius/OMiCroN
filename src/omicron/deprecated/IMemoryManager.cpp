#include "IMemoryManager.h"

namespace omicron
{
	unique_ptr< IMemoryManager> SingletonMemoryManager::m_instance = nullptr;
}
