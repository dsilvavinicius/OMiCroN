#ifndef GPU_ALLOC_STATISTICS_H
#define GPU_ALLOC_STATISTICS_H

#include "BasicTypes.h"
#include "splat_renderer/surfel.hpp"
#include <atomic>

namespace model
{
	class GpuAllocStatistics
	{
	public:
		static void notifyAlloc( const ulong size )
		{
			m_allocated += size;
		}
		
		static void notifyDealloc( const ulong size )
		{
			m_allocated -= size;
		}
	
		static ulong totalAllocated()
		{
			return m_allocated.load();
		}
	
		static ulong pointSize()
		{
			return sizeof( Surfel );
		}
	
	private:
		static atomic_ulong m_allocated;
	};
}

#endif