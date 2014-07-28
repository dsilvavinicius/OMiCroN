#include "Point.h"

namespace model
{
	Point::Point(const vec3& color, const vec3& pos)
	{
		m_color = make_shared<vec3>(color);
		m_pos = make_shared<vec3>(pos);
	}
	
	shared_ptr<vec3> Point::getColor() { return m_color; }
	shared_ptr<vec3> Point::getPos() { return m_pos; }
}