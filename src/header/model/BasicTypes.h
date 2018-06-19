#ifndef BASIC_TYPES_H
#define BASIC_TYPES_H

#include <memory>
#include <eigen3/Eigen/Dense>
#include "global_malloc.h"

using namespace std;
using namespace Eigen;

namespace model
{
	// Definition of library's basic types.
	using uint = unsigned int;
	using ulong = unsigned long;
	using uchar = unsigned char;
	using byte = uchar;
	using Float = float;
	
	using Vec2 = Vector2f;
	using Vec3 = Vector3f;
	using Vec3Ptr = shared_ptr< Vec3 >;
	using ConstVec3Ptr = shared_ptr< const Vec3 >;
}

#endif