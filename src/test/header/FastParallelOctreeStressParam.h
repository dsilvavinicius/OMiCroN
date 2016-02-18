#include "BasicTypes.h"
#include <string>

using namespace std;

typedef struct FastParallelOctreeStressParam
{
	FastParallelOctreeStressParam();
	FastParallelOctreeStressParam( const string& plyFilename, const int nThreads, const int hierarchyLvl,
								   const int workItemSize, const ulong memoryQuota );
	
	friend ostream& operator<<( ostream &out, const FastParallelOctreeStressParam &param );
	
	int m_nThreads;
	string m_plyFilename;
	int m_hierarchyLvl;
	int m_workItemSize;
	ulong m_memoryQuota;
} FastParallelOctreeStressParam;