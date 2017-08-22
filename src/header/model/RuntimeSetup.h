#ifndef RUNTIME_SETUP_H
#define RUNTIME_SETUP_H

namespace model
{
	typedef struct RuntimeSetup
	{
		RuntimeSetup( int nThreads = 8, ulong loadPerThread = 1024, ulong memoryQuota = 1024 * 1024 * 8 )
		: m_nThreads( nThreads ),
		m_loadPerThread( loadPerThread ),
		m_memoryQuota( memoryQuota )
		{}
		
		int m_nThreads;
		ulong m_loadPerThread;
		ulong m_memoryQuota;
	} RuntimeSetup;
}

#endif