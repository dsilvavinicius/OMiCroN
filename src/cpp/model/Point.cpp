#include "Point.h"
#include <MemoryManager.h>

namespace model
{
	Point::Point()
	: m_color( 0, 0, 0 ),
	m_pos( 0, 0, 0 )
	{}
	
	Point::Point( const Point& other )
	: Point( other.m_color, other.m_pos )
	{}
	
	Point& Point::operator=( const Point& other )
	{
		m_pos = other.m_pos;
		m_color = other.m_color;
		
		return *this;
	}
	
	Point::Point( const Vec3& color, const Vec3& pos )
	: m_color( color ),
	m_pos( pos )
	{}
	
	Point::Point( Point&& other )
	{
		m_color = other.m_color;
		m_pos = other.m_pos;
	}
	
	Point& Point::operator=( Point&& other )
	{
		if( this != &other )
		{
			m_color = other.m_color;
			m_pos = other.m_pos;
		}
		
		return *this;
	}
	
	Point::Point( byte* serialization )
	{
		size_t sizeOfVec3 = sizeof( Vec3 );
		memcpy( &m_color, serialization, sizeOfVec3 );
		memcpy( &m_pos, serialization + sizeOfVec3, sizeOfVec3 );
	}
	
	Point::Point( byte* serialization, byte*& pastRead )
	: Point( serialization )
	{
		pastRead = serialization + 2 * sizeof( Vec3 );
	}
	
	void* Point::operator new( size_t size )
	{
		return SingletonMemoryManager::instance().allocPoint();
	}
	
	void* Point::operator new[]( size_t size )
	{
		return SingletonMemoryManager::instance().allocPointArray( size );
	}
	
	void Point::operator delete( void* p )
	{
		SingletonMemoryManager::instance().deallocPoint( p );
	}
	
	void Point::operator delete[]( void* p )
	{
		SingletonMemoryManager::instance().deallocPointArray( p );
	}
	
	Vec3& Point::getColor() { return m_color; }
	
	const Vec3& Point::getColor() const { return m_color; }
	
	Vec3& Point::getPos() { return m_pos; }
	
	const Vec3& Point::getPos() const { return m_pos; }
	
	bool Point::equal( const Point& other, const Float& epsilon ) const
	{
		return 	glm::distance2( m_color, other.m_color ) < epsilon && glm::distance2( m_pos, other.m_pos ) < epsilon;
	}
	
	Point Point::multiply( const Float& multiplier ) const
	{
		return Point( m_color * multiplier, m_pos * multiplier );
	}
	
	Point operator+( const Point& left, const Point& right )
	{
		return Point( left.m_color + right.m_color, left.m_pos + right.m_pos );
	}
	
	Point operator+( Point&& left , const Point& right )
	{
		return Point( left.m_color + right.m_color, left.m_pos + right.m_pos );
	}
	
	Point operator+( const Point& left, Point&& right )
	{
		return Point( left.m_color + right.m_color, left.m_pos + right.m_pos );
	}
	
	Point operator+( Point&& left, Point&& right )
	{
		return Point( left.m_color + right.m_color, left.m_pos + right.m_pos );
	}
	
	ostream& operator<< ( ostream &out, const Point &point )
	{
		out << "Point:" << endl
			<< "color = " << glm::to_string( point.m_color ) << endl
			<< "pos = " << glm::to_string( point.m_pos ) << endl;
			
		return out;
	}
	
	size_t Point::serialize( byte** serialization ) const
	{
		size_t sizeOfVec3 = sizeof( Vec3 );
		size_t sizeOfPoint = 2 * sizeOfVec3;
		
		*serialization = new byte[ sizeOfPoint ];
		memcpy( *serialization, &m_color ,sizeOfVec3 );
		memcpy( *serialization + sizeOfVec3, &m_pos, sizeOfVec3 );
		
		return sizeOfPoint;
	}
}