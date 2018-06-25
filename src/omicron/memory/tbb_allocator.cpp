#include "omicron/memory/tbb_allocator.h"

namespace omicron::memory
{
	atomic_ulong AllocStatistics::m_allocated( 0 );
}
