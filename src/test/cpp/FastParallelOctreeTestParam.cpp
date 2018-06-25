#include <iostream>
#include "header/FastParallelOctreeTestParam.h"

FastParallelOctreeTestParam::FastParallelOctreeTestParam()
{}

FastParallelOctreeTestParam::FastParallelOctreeTestParam( const string& plyFilename, const int nThreads, const int hierarchyLvl,
														  const int workItemSize, const ulong memoryQuota )
: m_nThreads( nThreads ),
m_plyFilename( plyFilename ),
m_hierarchyLvl( hierarchyLvl ),
m_workItemSize( workItemSize ),
m_memoryQuota( memoryQuota )
{};

ostream& operator<<( ostream &out, const FastParallelOctreeTestParam &param )
{
	out << param.m_plyFilename << endl << "Threads: " << param.m_nThreads << endl << "Max lvl: "
		<< param.m_hierarchyLvl << endl << "Workitem size:" << param.m_workItemSize << endl
		<< "Mem quota (in bytes):" << param.m_memoryQuota << endl;
	return out;
}
