#include "omicron/hierarchy/gpu_alloc_statistics.h"
#include "omicron/hierarchy/reconstruction_params.h"

namespace omicron::hierarchy
{
	atomic_ulong GpuAllocStatistics::m_allocated( 0ul );
	
	ulong GpuAllocStatistics::m_totalGpuMem( GPU_MEMORY );
}
