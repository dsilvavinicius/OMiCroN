#include "omicron/hierarchy/hierarchy_creation_log.h"

namespace omicron::hierarchy
{
	recursive_mutex HierarchyCreationLog::m_logMutex;
 	ofstream HierarchyCreationLog::m_log( "HierarchyCreationLog.txt" );
}
