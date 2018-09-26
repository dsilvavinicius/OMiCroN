#ifndef EXTERNAL_VECTOR_TYPES_H
#define EXTERNAL_VECTOR_TYPES_H

#include <stxxl/vector>
#include "omicron/renderer/splat_renderer/surfel.hpp"

namespace omicron::hierarchy
{
    using namespace omicron::renderer;

    using ExtSurfelVector = stxxl::VECTOR_GENERATOR< Surfel >::result;
	using ExtIndexVector = stxxl::VECTOR_GENERATOR< ulong >::result;
}

#endif