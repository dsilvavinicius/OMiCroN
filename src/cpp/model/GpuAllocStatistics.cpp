#include "GpuAllocStatistics.h"

namespace model
{
	atomic_ulong GpuAllocStatistics::m_allocated( 0ul );
	
	ulong GpuAllocStatistics::m_totalGpuMem( 900ul * 1024ul * 1024ul );
}