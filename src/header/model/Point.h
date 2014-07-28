#ifndef POINT_H
#define POINT_H

#include <glm/glm.hpp>
#include <memory>

using namespace std;
using namespace glm;

namespace model
{
	class Point
	{
	public:
		Point(const vec3& color, const vec3& pos);
		
		shared_ptr<vec3> getColor();
		shared_ptr<vec3> getPos();
	private:
		shared_ptr<vec3> m_color;
		shared_ptr<vec3> m_pos;
	};
	
	using PointPtr = shared_ptr<Point>;
}

#endif