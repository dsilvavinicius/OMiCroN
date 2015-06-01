#include "Point.h"

namespace model
{
	Point::Point()
	{
		m_color = make_shared< Vec3 >( Vec3( 0, 0, 0 ) );
		m_pos = make_shared< Vec3 >( Vec3( 0, 0, 0 ) );
	}
	
	Point::Point( const Point& other )
	: Point( *other.m_color, *other.m_pos )
	{}
	
	Point& Point::operator=( const Point& other )
	{
		*m_pos = *other.m_pos;
		*m_color = *other.m_color;
		
		return *this;
	}
	
	Point::Point( const Vec3& color, const Vec3& pos )
	{
		m_color = make_shared< Vec3 >( color );
		m_pos = make_shared< Vec3 >( pos );
	}
	
	Point::Point( Point&& other )
	{
		m_color = other.m_color;
		m_pos = other.m_pos;
		
		other.m_color = nullptr;
		other.m_pos = nullptr;
	}
	
	Point& Point::operator=( Point&& other )
	{
		if (this!=&other)
		{
			m_color = other.m_color;
			m_pos = other.m_pos;
		
			other.m_color = nullptr;
			other.m_pos = nullptr;
		}
		
		return *this;
	}
	
	Point::Point( byte* serialization )
	{
		size_t sizeOfVec3 = sizeof( Vec3 );
		Vec3 color;
		memcpy( &color, serialization, sizeOfVec3 );
		Vec3 pos;
		memcpy( &pos, serialization + sizeOfVec3, sizeOfVec3 );
		
		m_color = make_shared< Vec3 >( color );
		m_pos = make_shared< Vec3 >( pos );
	}
	
	Vec3Ptr Point::getColor() { return m_color; }
	
	const ConstVec3Ptr Point::getColor() const { return m_color; }
	
	Vec3Ptr Point::getPos() { return m_pos; }
	
	const ConstVec3Ptr Point::getPos() const { return m_pos; }
	
	bool Point::equal( const Point& other, const Float& epsilon ) const
	{
		return 	glm::distance2( *m_color, *other.m_color ) < epsilon && glm::distance2( *m_pos, *other.m_pos ) < epsilon;
	}
	
	Point Point::multiply( const Float& multiplier ) const
	{
		return Point( *m_color * multiplier, *m_pos * multiplier );
	}
	
	Point operator+( const Point& left, const Point& right )
	{
		return Point( *left.m_color + *right.m_color, *left.m_pos + *right.m_pos );
	}
	
	Point operator+( Point&& left , const Point& right )
	{
		return Point( *left.m_color + *right.m_color, *left.m_pos + *right.m_pos );
	}
	
	Point operator+( const Point& left, Point&& right )
	{
		return Point( *left.m_color + *right.m_color, *left.m_pos + *right.m_pos );
	}
	
	Point operator+( Point&& left, Point&& right )
	{
		return Point( *left.m_color + *right.m_color, *left.m_pos + *right.m_pos );
	}
	
	ostream& operator<< ( ostream &out, const Point &point )
	{
		Vec3 pos = *point.m_pos;
		Vec3 color = *point.m_color;
		
		out << "Point:" << endl
			<< "color = " << glm::to_string( color ) << endl
			<< "pos = " << glm::to_string( pos ) << endl;
			
		return out;
	}
	
	size_t Point::serialize( byte** serialization ) const
	{
		size_t sizeOfVec3 = sizeof( Vec3 );
		size_t sizeOfPoint = 2 * sizeOfVec3;
		
		*serialization = new byte[ sizeOfPoint ];
		memcpy( *serialization, &( *m_color ) ,sizeOfVec3 );
		memcpy( *serialization + sizeOfVec3, &( *m_pos ), sizeOfVec3 );
		
		return sizeOfPoint;
	}
}