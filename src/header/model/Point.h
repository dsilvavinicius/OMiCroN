#ifndef POINT_H
#define POINT_H

#include <glm/glm.hpp>
#include <memory>

using namespace std;

namespace model
{
	class Point
	{
	public:
	private:
		vec3 color;
		vec3 pos;
	};
	
	using PointPtr = shared_ptr<Point>;
}

#endif