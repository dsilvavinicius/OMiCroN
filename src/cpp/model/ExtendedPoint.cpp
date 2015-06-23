#include "ExtendedPoint.h"

namespace model
{
	ExtendedPoint::ExtendedPoint()
	: Point::Point()
	{
		m_normal = make_shared< Vec3 >( Vec3( 0, 0, 0 ) );
	}
	
	ExtendedPoint::ExtendedPoint( const ExtendedPoint& other )
	: ExtendedPoint( *other.m_color, *other.m_normal, *other.m_pos )
	{}
	
	ExtendedPoint& ExtendedPoint::operator=( const ExtendedPoint& other )
	{
		Point::operator=( other );
		*m_normal = *other.m_normal;
		
		return *this;
	}
	
	ExtendedPoint::ExtendedPoint( const Vec3& color, const Vec3& normal, const Vec3& pos )
	: Point( color, pos )
	{
		m_normal = make_shared< Vec3 >( normal );
	}
	
	ExtendedPoint::ExtendedPoint( ExtendedPoint&& other )
	: Point( other )
	{
		m_normal = other.m_normal;
		other.m_normal = nullptr;
	}
	
	ExtendedPoint& ExtendedPoint::operator=( ExtendedPoint&& other )
	{
		Point::operator=( other );
		if (this!=&other)
		{
			m_normal = other.m_normal;
			other.m_normal = nullptr;
		}
		
		return *this;
	}
	
	ExtendedPoint::ExtendedPoint( byte* serialization )
	: Point::Point( serialization )
	{
		size_t sizeOfVec3 = sizeof( Vec3 );
		Vec3 normal;
		memcpy( &normal, serialization + 2 * sizeOfVec3, sizeOfVec3 );
		
		m_normal = make_shared< Vec3 >( normal );
	}
	
	ExtendedPoint::ExtendedPoint( byte* serialization, byte*& pastRead )
	: ExtendedPoint( serialization )
	{
		pastRead = serialization + 3 * sizeof( Vec3 );
	}
	
	shared_ptr< Vec3 > ExtendedPoint::getNormal() { return m_normal; }
	
	const shared_ptr< const Vec3 > ExtendedPoint::getNormal() const { return m_normal; }
	
	bool ExtendedPoint::equal( const ExtendedPoint& other, const float& epsilon ) const
	{
		return Point::equal( other, epsilon ) && glm::distance2( *m_normal, *other.m_normal ) < epsilon;
	}
	
	ExtendedPoint ExtendedPoint::multiply( const Float& multiplier ) const
	{
		return ExtendedPoint( *Point::m_color * multiplier, *m_normal * multiplier, *Point::m_pos * multiplier );
	}
	
	ExtendedPoint operator+( const ExtendedPoint& left, const ExtendedPoint& right )
	{
		return ExtendedPoint( *left.m_color + *right.m_color, *left.m_normal + *right.m_normal, *left.m_pos + *right.m_pos );
	}
	
	ExtendedPoint operator+( ExtendedPoint&& left , const ExtendedPoint& right )
	{
		return ExtendedPoint( *left.m_color + *right.m_color, *left.m_normal + *right.m_normal, *left.m_pos + *right.m_pos );
	}
	
	ExtendedPoint operator+( const ExtendedPoint& left, ExtendedPoint&& right )
	{
		return ExtendedPoint( *left.m_color + *right.m_color, *left.m_normal + *right.m_normal, *left.m_pos + *right.m_pos );
	}
	
	ExtendedPoint operator+( ExtendedPoint&& left, ExtendedPoint&& right )
	{
		return ExtendedPoint( *left.m_color + *right.m_color, *left.m_normal + *right.m_normal, *left.m_pos + *right.m_pos);
	}
	
	ostream& operator<<( ostream &out, const ExtendedPoint &point )
	{
		Vec3 pos = *point.m_pos;
		Vec3 color = *point.m_color;
		Vec3 normal = *point.m_normal;
		
		out << "Point:" << endl
			<< "pos = " << glm::to_string( pos ) << endl
			<< "color = " << glm::to_string( color ) << endl
			<< "normal = " << glm::to_string( normal ) << endl;
			
		return out;
	}
	
	size_t ExtendedPoint::serialize( byte** serialization ) const
	{
		size_t sizeOfVec3 = sizeof( Vec3 );
		size_t sizeOfPoint = 3 * sizeOfVec3;
		
		*serialization = new byte[ sizeOfPoint ];
		memcpy( *serialization, &( *Point::m_color ) ,sizeOfVec3 );
		memcpy( *serialization + sizeOfVec3, &( *Point::m_pos ), sizeOfVec3 );
		memcpy( *serialization + 2 * sizeOfVec3, &( *m_normal ), sizeOfVec3 );
		
		return sizeOfPoint;
	}
}