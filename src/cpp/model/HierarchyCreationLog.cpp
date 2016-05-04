#include "HierarchyCreationLog.h"

namespace model
{
	recursive_mutex HierarchyCreationLog::m_logMutex;
 	ofstream HierarchyCreationLog::m_log( "HierarchyCreationLog.txt" );
}