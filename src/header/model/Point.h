#ifndef POINT_H
#define POINT_H

#include <glm/glm.hpp>
#include <memory>

using namespace std;
using namespace glm;

namespace model
{
	/** Point representation. Float is used as the scalar type and glm Vec3 type is used as the 3-dimension vector. */
	template < typename Float, typename Vec3 >
	class Point
	{
	public:
		Point(const Vec3& color, const Vec3& pos);
		
		Point< Float, Vec3 > operator+(const Point< Float, Vec3 >& other);
		Point< Float, Vec3 > operator*(const Float& number);
		
		shared_ptr< Vec3 > getColor();
		shared_ptr< Vec3 > getPos();
	private:
		shared_ptr< Vec3 > m_color;
		shared_ptr< Vec3 > m_pos;
	};
	
	template <typename Float, typename Vec3>
	Point< Float, Vec3 >::Point(const Vec3& color, const Vec3& pos)
	{
		m_color = make_shared< Vec3 >(color);
		m_pos = make_shared< Vec3 >(pos);
	}
	
	template <typename Float, typename Vec3>
	void Point< Float, Vec3 >::operator+(const Point< Float, Vec3 >& other)
	{
		*m_color += other.getColor();
		*m_pos += other.getPos();
	}
	
	template <typename Float, typename Vec3>
	void Point< Float, Vec3 >::operator*(const Float& number)
	{
		*m_color *= number;
		*m_pos *= number;
	}
	
	template <typename Float, typename Vec3>
	shared_ptr< Vec3 > Point< Float, Vec3 >::getColor() { return m_color; }
	
	template <typename Float, typename Vec3>
	shared_ptr< Vec3 > Point< Float, Vec3 >::getPos() { return m_pos; }
	
	/** Point smart pointer. */
	template <typename Float, typename Vec3>
	using PointPtr = shared_ptr< Point< Float, Vec3 > >;
	
	/** Vector of Points. */
	template <typename Float, typename Vec3>
	using PointVector = vector< PointPtr< Float, Vec3 > >;
	
	/** Ptr for Vector of Points. */
	template <typename Float, typename Vec3>
	using PointVectorPtr = shared_ptr< PointVector< Float, Vec3 > >;
}

#endif