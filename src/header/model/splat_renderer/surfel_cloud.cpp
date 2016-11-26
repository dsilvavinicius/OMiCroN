#include "splat_renderer/surfel_cloud.h"

// #define BUFFER_MTX 

#ifdef BUFFER_MTX
	mutex SurfelCloud::m_bufferBindMtx;
#endif

#undef BUFFER_MTX