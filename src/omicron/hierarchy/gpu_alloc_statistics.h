#ifndef GPU_ALLOC_STATISTICS_H
#define GPU_ALLOC_STATISTICS_H

#include "omicron/basic/basic_types.h"
#include "omicron/renderer/splat_renderer/surfel.hpp"
#include "omicron/basic/array.h"
#include <atomic>

namespace omicron::hierarchy
{
    using namespace omicron::basic;
    
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
		
		static bool hasMemoryFor( const Array< Surfel >& points )
		{
			ulong neededGpuMem = pointSize() * points.size();
			return totalAllocated() + neededGpuMem < m_totalGpuMem;
		}
		
		static bool reachedGpuMemQuota()
		{
			return float( totalAllocated() ) > 0.1f * float( m_totalGpuMem );
		}
	
	private:
		static atomic_ulong m_allocated;
		static ulong m_totalGpuMem;
	};
}

#endif
