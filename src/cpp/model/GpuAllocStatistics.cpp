#include "GpuAllocStatistics.h"
#include "ReconstructionParams.h"

namespace model
{
	atomic_ulong GpuAllocStatistics::m_allocated( 0ul );
	
	ulong GpuAllocStatistics::m_totalGpuMem( GPU_MEMORY );
}