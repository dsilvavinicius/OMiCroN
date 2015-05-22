#ifndef EXTENDED_POINT_H
#define EXTENDED_POINT_H

#include <memory>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "Point.h"

using namespace std;

namespace model
{
	/** Point with normal. */
	template< typename Float, typename Vec3 >
	class ExtendedPoint : public Point< Float, Vec3 >
	{
		using Point = model::Point< Float, Vec3 >;
	public:
		ExtendedPoint();
		ExtendedPoint( const ExtendedPoint& other );
		ExtendedPoint& operator=( const ExtendedPoint& other );
		ExtendedPoint( const Vec3& color, const Vec3& normal, const Vec3& pos );
		ExtendedPoint( ExtendedPoint&& other );
		ExtendedPoint& operator=( ExtendedPoint&& other);
		ExtendedPoint( byte* serialization );
		
		shared_ptr< Vec3 > getNormal();
		
		// Comparison operators.
		bool equal( const ExtendedPoint< Float, Vec3 >& other, const float& epsilon ) const;
		
		// Arithmetic operators.
		ExtendedPoint< Float, Vec3 > multiply( const Float& multiplier ) const;
		
		template< typename F, typename V >
		friend ExtendedPoint< F, V > operator+( const ExtendedPoint< F, V >& left, const ExtendedPoint< F, V >& right );
		
		template< typename F, typename V >
		friend ExtendedPoint< F, V > operator+( ExtendedPoint< F, V >&& left , const ExtendedPoint< F, V >& right );
		
		template< typename F, typename V >
		friend ExtendedPoint< F, V > operator+( const ExtendedPoint< F, V >& left, ExtendedPoint< F, V >&& right );
		
		template< typename F, typename V >
		friend ExtendedPoint< F, V > operator+( ExtendedPoint< F, V >&& left, ExtendedPoint< F, V >&& right );
		
		template< typename F, typename V >
		friend ostream& operator<<( ostream &out, const ExtendedPoint< F, V >& point );
		
		size_t serialize( byte** serialization ) const;
		
	protected:
		shared_ptr< Vec3 > m_normal;
	};
	
	template <typename Float, typename Vec3>
	ExtendedPoint< Float, Vec3 >::ExtendedPoint()
	: Point::Point()
	{
		m_normal = make_shared< Vec3 >( Vec3( 0, 0, 0 ) );
	}
	
	template< typename Float, typename Vec3 >
	ExtendedPoint< Float, Vec3 >::ExtendedPoint( const ExtendedPoint& other )
	: ExtendedPoint( *other.m_color, *other.m_normal, *other.m_pos )
	{}
	
	template <typename Float, typename Vec3>
	ExtendedPoint< Float, Vec3 >& ExtendedPoint< Float, Vec3 >::operator=( const ExtendedPoint& other )
	{
		Point::operator=( other )
		*m_normal = *other.m_normal;
		
		return *this;
	}
	
	template <typename Float, typename Vec3>
	ExtendedPoint< Float, Vec3 >::ExtendedPoint( const Vec3& color, const Vec3& normal, const Vec3& pos )
	: Point( color, pos )
	{
		m_normal = make_shared< Vec3 >( normal );
	}
	
	template <typename Float, typename Vec3>
	ExtendedPoint< Float, Vec3 >::ExtendedPoint( ExtendedPoint&& other )
	: Point( other )
	{
		m_normal = other.m_normal;
		other.m_normal = nullptr;
	}
	
	template <typename Float, typename Vec3>
	ExtendedPoint< Float, Vec3 >& ExtendedPoint< Float, Vec3 >::operator=( ExtendedPoint&& other )
	{
		Point::operator=( other );
		if (this!=&other)
		{
			m_normal = other.m_normal;
			other.m_normal = nullptr;
		}
		
		return *this;
	}
	
	template <typename Float, typename Vec3>
	ExtendedPoint< Float, Vec3 >::ExtendedPoint( byte* serialization )
	: Point::Point( serialization )
	{
		size_t sizeOfVec3 = sizeof( Vec3 );
		Vec3 normal;
		memcpy( &normal, serialization + 2 * sizeOfVec3, sizeOfVec3 );
		
		m_normal = make_shared< Vec3 >( normal );
	}
	
	template <typename Float, typename Vec3>
	shared_ptr< Vec3 > ExtendedPoint< Float, Vec3 >::getNormal() { return m_normal; }
	
	template <typename Float, typename Vec3>
	bool ExtendedPoint< Float, Vec3 >::equal( const ExtendedPoint< Float, Vec3 >& other, const float& epsilon ) const
	{
		return Point::equal( other, epsilon ) && glm::distance2( *m_normal, *other.m_normal ) < epsilon;
	}
	
	template <typename Float, typename Vec3>
	ExtendedPoint< Float, Vec3 > ExtendedPoint< Float, Vec3 >::multiply( const Float& multiplier ) const
	{
		return ExtendedPoint< Float, Vec3 >( *Point::m_color * multiplier, *m_normal * multiplier,
											 *Point::m_pos * multiplier );
	}
	
	template < typename Float, typename Vec3 >
	ExtendedPoint< Float, Vec3 > operator+( const ExtendedPoint< Float, Vec3 >& left,
											const ExtendedPoint< Float, Vec3 >& right )
	{
		return ExtendedPoint< Float, Vec3 >( *left.m_color + *right.m_color, *left.m_normal + *right.m_normal,
											 *left.m_pos + *right.m_pos );
	}
	
	template < typename Float, typename Vec3 >
	ExtendedPoint< Float, Vec3 > operator+( ExtendedPoint< Float, Vec3 >&& left , const ExtendedPoint< Float, Vec3 >& right )
	{
		return ExtendedPoint< Float, Vec3 >( *left.m_color + *right.m_color, *left.m_normal + *right.m_normal,
											 *left.m_pos + *right.m_pos );
	}
	
	template < typename Float, typename Vec3 >
	ExtendedPoint< Float, Vec3 > operator+( const ExtendedPoint< Float, Vec3 >& left, ExtendedPoint< Float, Vec3 >&& right )
	{
		return ExtendedPoint< Float, Vec3 >( *left.m_color + *right.m_color, *left.m_normal + *right.m_normal,
											 *left.m_pos + *right.m_pos );
	}
	
	template < typename Float, typename Vec3 >
	ExtendedPoint< Float, Vec3 > operator+( ExtendedPoint< Float, Vec3 >&& left, ExtendedPoint< Float, Vec3 >&& right )
	{
		return ExtendedPoint< Float, Vec3 >( *left.m_color + *right.m_color, *left.m_normal + *right.m_normal,
											 *left.m_pos + *right.m_pos);
	}
	
	template< typename Float, typename Vec3 >
	ostream& operator<<( ostream &out, const ExtendedPoint< Float, Vec3 > &point )
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
	
	template < typename Float, typename Vec3 >
	size_t ExtendedPoint< Float, Vec3 >::serialize( byte** serialization ) const
	{
		size_t sizeOfVec3 = sizeof( Vec3 );
		size_t sizeOfPoint = 3 * sizeOfVec3;
		
		*serialization = new byte[ sizeOfPoint ];
		memcpy( *serialization, &( *Point::m_color ) ,sizeOfVec3 );
		memcpy( *serialization + sizeOfVec3, &( *Point::m_pos ), sizeOfVec3 );
		memcpy( *serialization + 2 * sizeOfVec3, &( *m_normal ), sizeOfVec3 );
		
		return sizeOfPoint;
	}
	
	//===========
	// Type sugar
	//===========
	
	/** ExtendedPoint smart pointer. */
	template <typename Float, typename Vec3>
	using ExtendedPointPtr = shared_ptr< ExtendedPoint< Float, Vec3 > >;
	
	/** Vector of ExtendedPoints. */
	template <typename Float, typename Vec3>
	using ExtendedPointVector = vector< ExtendedPointPtr< Float, Vec3 > >;
	
	/** Ptr for Vector of ExtendedPoints. */
	template <typename Float, typename Vec3>
	using ExtendedPointVectorPtr = shared_ptr< ExtendedPointVector< Float, Vec3 > >;
}

#endif