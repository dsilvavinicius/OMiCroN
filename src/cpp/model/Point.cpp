#include "Point.h"
#include <Serializer.h>

namespace model
{
	Point::Point()
	: m_normal( 0, 0, 0 ),
	m_pos( 0, 0, 0 )
	{}
	
	Point::Point( const Point& other )
	: Point( other.m_normal, other.m_pos )
	{}
	
	Point& Point::operator=( const Point& other )
	{
		m_pos = other.m_pos;
		m_normal = other.m_normal;
		
		return *this;
	}
	
	Point::Point( const Vec3& color, const Vec3& pos )
	: m_normal( color ),
	m_pos( pos )
	{}
	
	Point::Point( Point&& other )
	{
		m_normal = other.m_normal;
		m_pos = other.m_pos;
	}
	
	Point& Point::operator=( Point&& other )
	{
		if( this != &other )
		{
			m_normal = other.m_normal;
			m_pos = other.m_pos;
		}
		
		return *this;
	}
	
	Point::Point( byte* serialization )
	{
		size_t sizeOfVec3 = sizeof( Vec3 );
		memcpy( &m_normal, serialization, sizeOfVec3 );
		memcpy( &m_pos, serialization + sizeOfVec3, sizeOfVec3 );
	}
	
	Point::Point( byte* serialization, byte*& pastRead )
	: Point( serialization )
	{
		pastRead = serialization + 2 * sizeof( Vec3 );
	}
	
	void* Point::operator new( size_t size )
	{
		return ManagedAllocator< Point >().allocate( 1 );
	}
	
	void* Point::operator new[]( size_t size )
	{
		return ManagedAllocator< Point >().allocate( size / sizeof( Point ) );
	}
	
	void Point::operator delete( void* p )
	{
		ManagedAllocator< Point >().deallocate(
			static_cast< ManagedAllocator< Point >::pointer >( p ), 1
		);
	}
	
	void Point::operator delete[]( void* p )
	{
		ManagedAllocator< Point >().deallocate(
			static_cast< ManagedAllocator< Point >::pointer >( p ), 2
		);
	}
	
	Vec3& Point::getNormal() { return m_normal; }
	
	const Vec3& Point::getNormal() const { return m_normal; }
	
	Vec3& Point::getPos() { return m_pos; }
	
	const Vec3& Point::getPos() const { return m_pos; }
	
	bool Point::equal( const Point& other, const Float& epsilon ) const
	{
		return m_normal.isApprox( other.m_normal, epsilon ) && m_pos.isApprox( other.m_pos, epsilon );
	}
	
	Point Point::multiply( const Float& multiplier ) const
	{
		return Point( m_normal * multiplier, m_pos * multiplier );
	}
	
	Point operator+( const Point& left, const Point& right )
	{
		return Point( left.m_normal + right.m_normal, left.m_pos + right.m_pos );
	}
	
	Point operator+( Point&& left , const Point& right )
	{
		return Point( left.m_normal + right.m_normal, left.m_pos + right.m_pos );
	}
	
	Point operator+( const Point& left, Point&& right )
	{
		return Point( left.m_normal + right.m_normal, left.m_pos + right.m_pos );
	}
	
	Point operator+( Point&& left, Point&& right )
	{
		return Point( left.m_normal + right.m_normal, left.m_pos + right.m_pos );
	}
	
	ostream& operator<< ( ostream &out, const Point &point )
	{
		out << "Point:" << endl
			<< "color = " << point.m_normal << endl
			<< "pos = " << point.m_pos << endl;
			
		return out;
	}
	
	size_t Point::serialize( byte** serialization ) const
	{
		size_t sizeOfVec3 = sizeof( Vec3 );
		size_t sizeOfPoint = 2 * sizeOfVec3;
		
		*serialization = Serializer::newByteArray( sizeOfPoint );
		memcpy( *serialization, &m_normal ,sizeOfVec3 );
		memcpy( *serialization + sizeOfVec3, &m_pos, sizeOfVec3 );
		
		return sizeOfPoint;
	}
}