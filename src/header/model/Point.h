#ifndef POINT_H
#define POINT_H

#include <glm/glm.hpp>
#include <memory>

using namespace std;
using namespace glm;

namespace model
{
	template <typename VecType>
	class Point
	{
	public:
		Point(const vec3& color, const vec3& pos);
		
		shared_ptr<VecType> getColor();
		shared_ptr<VecType> getPos();
	private:
		shared_ptr<VecType> m_color;
		shared_ptr<VecType> m_pos;
	};
	
	/** Point: high precision. */
	template <typename NumType>
	using PointHP = Point< tvec3<NumType, highp> >;
	
	/** Point: medium precision. */
	template <typename NumType>
	using PointMP = Point< tvec3<NumType, mediump> >;
	
	/** Point: low precision. */
	template <typename NumType>
	using PointLP = Point< tvec3<NumType, lowp> >;
	
	/** Point smart pointer. */
	template <typename NumType>
	using PointPtr = shared_ptr< Point< tvec3<NumType, lowp> > >;
}

#endif