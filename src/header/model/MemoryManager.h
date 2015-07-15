#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include "MemoryPool.h"

namespace model
{
	/** Utilizes Memory pools in order to manage creation of managed objects. */
	class MemoryManager
	{
	public:
		MemoryManager();
		
	private:
		MemoryPool m_pointPool;
		MemoryPool m_nodePool;
	};
	
	inline MemoryManager::MemoryManager()
	{
		m_pointPool.createPool(  );
	}

}

#endif