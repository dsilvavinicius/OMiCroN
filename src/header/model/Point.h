#ifndef POINT_H
#define POINT_H

#include <glm/glm.hpp>
#include <memory>

using namespace std;
using namespace glm;

namespace model
{
	/** Point representation. The precision is specified by the glm Vec3 type. */
	template < typename Vec3 >
	class Point
	{
	public:
		Point(const Vec3& color, const Vec3& pos);
		
		shared_ptr< Vec3 > getColor();
		shared_ptr< Vec3 > getPos();
	private:
		shared_ptr< Vec3 > m_color;
		shared_ptr< Vec3 > m_pos;
	};
	
	/** Point smart pointer. */
	template <typename Vec3>
	using PointPtr = shared_ptr< Point< Vec3 > >;
}

#endif