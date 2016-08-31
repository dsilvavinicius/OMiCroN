#ifndef GPU_ALLOC_STATISTICS_H
#define GPU_ALLOC_STATISTICS_H

#include "BasicTypes.h"
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
			return 7ul * sizeof( float ); // 4 floats for positions and 3 for normals.;
		}
		
		static ulong extendedPointSize()
		{
			return 11ul * sizeof( float ); // 4 floats for positions, 4 for colors and 3 for normals.
		}
	
	private:
		static atomic_ulong m_allocated;
	};
}

#endif