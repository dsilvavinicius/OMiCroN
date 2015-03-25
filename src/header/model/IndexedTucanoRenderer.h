#ifndef INDEXED_TUCANO_RENDERER_H
#define INDEXED_TUCANO_RENDERER_H

#include "TucanoRenderingState.h"

namespace model
{
	/** Tucano renderer that sends all points to device at initialization time. After that, just sends indices to indicate
	 * 	which points should be rendered. */
	template< typename Vec3, typename Float >
	class IndexedTucanoRenderer
	: public TucanoRenderingState< Vec3, Float >
	{
		TucanoRenderingState( Trackball& camTrackball, Trackball& lightTrackball , Mesh& mesh, const Attributes& attribs,
							  const string& shaderPath, const Effect& effect = PHONG );
	};
}

#endif