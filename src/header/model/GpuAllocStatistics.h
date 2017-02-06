#ifndef GPU_ALLOC_STATISTICS_H
#define GPU_ALLOC_STATISTICS_H

#include "BasicTypes.h"
#include "splat_renderer/surfel.hpp"
#include "Array.h"
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
		
		static bool hasMemoryFor( const model::Array< Surfel >& points )
		{
			ulong neededGpuMem = pointSize() * points.size();
			return totalAllocated() + neededGpuMem < m_totalGpuMem;
		}
		
		static bool reachedGpuMemQuota()
		{
			return float( totalAllocated() ) > 0.8f * float( m_totalGpuMem );
		}
	
	private:
		static atomic_ulong m_allocated;
		static ulong m_totalGpuMem;
	};
}

#endif