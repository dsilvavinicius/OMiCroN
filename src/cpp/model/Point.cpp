#include "Point.h"

namespace model
{
	template <typename Vec3>
	Point<Vec3>::Point(const Vec3& color, const Vec3& pos)
	{
		m_color = make_shared<Vec3>(color);
		m_pos = make_shared<Vec3>(pos);
	}
	
	template <typename Vec3>
	shared_ptr<Vec3> Point<Vec3>::getColor() { return m_color; }
	
	template <typename Vec3>
	shared_ptr<Vec3> Point<Vec3>::getPos() { return m_pos; }
}