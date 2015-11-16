#ifndef BASIC_TYPES_H
#define BASIC_TYPES_H

#include <glm/glm.hpp>
#include <memory>
#include "Array.h"

using namespace std;
using namespace glm;

namespace model
{
	// Definition of library's basic types.
	using uint = unsigned int;
	using ulong = unsigned long;
	using uchar = unsigned char;
	using Float = float;
	
	using Vec3 = vec3;
	using Vec3Ptr = shared_ptr< Vec3 >;
	using ConstVec3Ptr = shared_ptr< const Vec3 >;
}

#endif