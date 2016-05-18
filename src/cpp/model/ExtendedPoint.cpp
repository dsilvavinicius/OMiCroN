#include "ExtendedPoint.h"
#include <MemoryManager.h>
#include <Serializer.h>

namespace model
{
	ExtendedPoint::ExtendedPoint()
	: Point::Point(),
	m_normal( 0, 0, 0 )
	{}
	
	ExtendedPoint::ExtendedPoint( const ExtendedPoint& other )
	: ExtendedPoint( other.m_color, other.m_normal, other.m_pos )
	{}
	
	ExtendedPoint& ExtendedPoint::operator=( const ExtendedPoint& other )
	{
		Point::operator=( other );
		m_normal = other.m_normal;
		
		return *this;
	}
	
	ExtendedPoint::ExtendedPoint( const Vec3& color, const Vec3& normal, const Vec3& pos )
	: Point( color, pos ),
	m_normal( normal )
	{}
	
	ExtendedPoint::ExtendedPoint( ExtendedPoint&& other )
	: Point( other )
	{
		m_normal = other.m_normal;
	}
	
	ExtendedPoint& ExtendedPoint::operator=( ExtendedPoint&& other )
	{
		Point::operator=( other );
		if( this != &other )
		{
			m_normal = other.m_normal;
		}
		
		return *this;
	}
	
	ExtendedPoint::ExtendedPoint( byte* serialization )
	: Point::Point( serialization )
	{
		size_t sizeOfVec3 = sizeof( Vec3 );
		memcpy( &m_normal, serialization + 2 * sizeOfVec3, sizeOfVec3 );
	}
	
	ExtendedPoint::ExtendedPoint( byte* serialization, byte*& pastRead )
	: ExtendedPoint( serialization )
	{
		pastRead = serialization + 3 * sizeof( Vec3 );
	}
	
	void* ExtendedPoint::operator new( size_t size )
	{
		return ManagedAllocator< ExtendedPoint >().allocate( 1 );
	}
	
	void* ExtendedPoint::operator new[]( size_t size )
	{
		return ManagedAllocator< ExtendedPoint >().allocate( size / sizeof( ExtendedPoint ) );
	}
	
	void ExtendedPoint::operator delete( void* p )
	{
		ManagedAllocator< ExtendedPoint >().deallocate(
			static_cast< ManagedAllocator< ExtendedPoint >::pointer >( p ), 1
		);
	}
	
	void ExtendedPoint::operator delete[]( void* p )
	{
		ManagedAllocator< ExtendedPoint >().deallocate(
			static_cast< ManagedAllocator< ExtendedPoint >::pointer >( p ), 2
		);
	}
	
	Vec3& ExtendedPoint::getNormal() { return m_normal; }
	
	const Vec3& ExtendedPoint::getNormal() const { return m_normal; }
	
	bool ExtendedPoint::equal( const ExtendedPoint& other, const float& epsilon ) const
	{
		return Point::equal( other, epsilon ) && m_normal.isApprox( other.m_normal, epsilon );
	}
	
	ExtendedPoint ExtendedPoint::multiply( const Float& multiplier ) const
	{
		return ExtendedPoint( Point::m_color * multiplier, m_normal * multiplier, Point::m_pos * multiplier );
	}
	
	ExtendedPoint operator+( const ExtendedPoint& left, const ExtendedPoint& right )
	{
		return ExtendedPoint( left.m_color + right.m_color, left.m_normal + right.m_normal, left.m_pos + right.m_pos );
	}
	
	ExtendedPoint operator+( ExtendedPoint&& left , const ExtendedPoint& right )
	{
		return ExtendedPoint( left.m_color + right.m_color, left.m_normal + right.m_normal, left.m_pos + right.m_pos );
	}
	
	ExtendedPoint operator+( const ExtendedPoint& left, ExtendedPoint&& right )
	{
		return ExtendedPoint( left.m_color + right.m_color, left.m_normal + right.m_normal, left.m_pos + right.m_pos );
	}
	
	ExtendedPoint operator+( ExtendedPoint&& left, ExtendedPoint&& right )
	{
		return ExtendedPoint( left.m_color + right.m_color, left.m_normal + right.m_normal, left.m_pos + right.m_pos );
	}
	
	ostream& operator<<( ostream &out, const ExtendedPoint &point )
	{
		out << "Point:" << endl
			<< "pos = " << point.m_pos << endl
			<< "color = " << point.m_color << endl
			<< "normal = " << point.m_normal << endl;
			
		return out;
	}
	
	size_t ExtendedPoint::serialize( byte** serialization ) const
	{
		size_t sizeOfVec3 = sizeof( Vec3 );
		size_t sizeOfPoint = 3 * sizeOfVec3;
		
		Vec3 color = Point::m_color;
		Vec3 pos = Point::m_pos;
		Vec3 normal = m_normal;
		
		*serialization = Serializer::newByteArray( sizeOfPoint );
		memcpy( *serialization, &color ,sizeOfVec3 );
		memcpy( *serialization + sizeOfVec3, &pos, sizeOfVec3 );
		memcpy( *serialization + 2 * sizeOfVec3, &normal, sizeOfVec3 );
		
		return sizeOfPoint;
	}
}