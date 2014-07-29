#ifndef NUMERIC_TYPE_H
#define NUMERIC_TYPE_H

#include "glm/glm.hpp"

using namespace glm;
using namespace glm::detail::lowp

namespace model
{
	/** The floating point number representation. */
	template <typename Type, typename Precision>
	struct Float;
	
	/** 32 bits Morton code. Octrees can reach 10 levels max. */
	template <> struct Float<float, >
	{
		using type = lowp_float;
	};
	
	/** The floating point vector representation. */
	template <typename Type, typename Precision>
	struct Float;
	
	/** 32 bits Morton code. Octrees can reach 10 levels max. */
	template <> struct Float<float, >
	{
		typedef int type;
	};
}
	

#endif