#include "omicron/hierarchy/external_octree_data.h"

namespace omicron::hierarchy
{
    ExtOctreeData::ExtSurfelVector ExtOctreeData::m_surfels;
	ExtOctreeData::ExtIndexVector ExtOctreeData::m_indices;
    mutex ExtOctreeData::m_indexMutex;
}