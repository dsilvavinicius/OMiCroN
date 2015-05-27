#ifndef POINT_H
#define POINT_H

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <memory>
#include <vector>
#include <iostream>

using namespace std;
using namespace glm;

namespace model
{
	/** Point representation. Float is used as the scalar type and glm Vec3 type is used as the 3-dimension vector. */
	template< typename Float, typename Vec3 >
	class Point
	{
	public:
		Point();
		Point( const Point& other );
		Point& operator=( const Point& other );
		Point( const Vec3& color, const Vec3& pos );
		Point( Point&& other );
		Point& operator=( Point&& other);
		
		/** Deserialization constructor.
		 *	@param serialization must be acquired by the serialize() method.*/
		Point( byte* serialization );
		
		shared_ptr< Vec3 > getColor();
		shared_ptr< Vec3 > getPos();
		
		const shared_ptr< const Vec3 > getColor() const;
		const shared_ptr< const Vec3 > getPos() const;
		
		// Comparison operators.
		bool equal( const Point< Float, Vec3 >& other, const float& epsilon ) const;
		
		// Arithmetic operators.
		Point< Float, Vec3 > multiply( const Float& multiplier ) const;
		
		template < typename F, typename V >
		friend Point< F, V > operator+( const Point< F, V >& left, const Point< F, V >& right );
		
		template < typename F, typename V >
		friend Point< F, V > operator+( Point< F, V >&& left , const Point< F, V >& right );
		
		template < typename F, typename V >
		friend Point< F, V > operator+( const Point< F, V >& left, Point< F, V >&& right );
		
		template < typename F, typename V >
		friend Point< F, V > operator+( Point< F, V >&& left, Point< F, V >&& right );
		
		template < typename F, typename V >
		friend ostream& operator<< ( ostream &out, const Point< F, V >& point );
		
		/** Serializes the point. The caller is responsible to delete the memory.
		 *  @param serialization is a pointer that will have the byte array at method return.
		 *  @returns the size of the byte array. */
		size_t serialize( byte** serialization ) const;
		
	protected:
		shared_ptr< Vec3 > m_color;
		shared_ptr< Vec3 > m_pos;
	};
	
	template <typename Float, typename Vec3>
	Point< Float, Vec3 >::Point()
	{
		m_color = make_shared< Vec3 >( Vec3( 0, 0, 0 ) );
		m_pos = make_shared< Vec3 >( Vec3( 0, 0, 0 ) );
	}
	
	template <typename Float, typename Vec3>
	Point< Float, Vec3 >::Point( const Point& other )
	: Point( *other.m_color, *other.m_pos )
	{}
	
	template <typename Float, typename Vec3>
	Point< Float, Vec3 >& Point< Float, Vec3 >::operator=( const Point& other )
	{
		*m_pos = *other.m_pos;
		*m_color = *other.m_color;
		
		return *this;
	}
	
	template <typename Float, typename Vec3>
	Point< Float, Vec3 >::Point( const Vec3& color, const Vec3& pos )
	{
		m_color = make_shared< Vec3 >( color );
		m_pos = make_shared< Vec3 >( pos );
	}
	
	template <typename Float, typename Vec3>
	Point< Float, Vec3 >::Point( Point&& other )
	{
		m_color = other.m_color;
		m_pos = other.m_pos;
		
		other.m_color = nullptr;
		other.m_pos = nullptr;
	}
	
	template <typename Float, typename Vec3>
	Point< Float, Vec3 >& Point< Float, Vec3 >::operator=( Point&& other )
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
	
	template <typename Float, typename Vec3>
	Point< Float, Vec3 >::Point( byte* serialization )
	{
		size_t sizeOfVec3 = sizeof( Vec3 );
		Vec3 color;
		memcpy( &color, serialization, sizeOfVec3 );
		Vec3 pos;
		memcpy( &pos, serialization + sizeOfVec3, sizeOfVec3 );
		
		m_color = make_shared< Vec3 >( color );
		m_pos = make_shared< Vec3 >( pos );
	}
	
	template <typename Float, typename Vec3>
	shared_ptr< Vec3 > Point< Float, Vec3 >::getColor() { return m_color; }
	
	template <typename Float, typename Vec3>
	const shared_ptr< const Vec3 > Point< Float, Vec3 >::getColor() const { return m_color; }
	
	template <typename Float, typename Vec3>
	shared_ptr< Vec3 > Point< Float, Vec3 >::getPos() { return m_pos; }
	
	template <typename Float, typename Vec3>
	const shared_ptr< const Vec3 > Point< Float, Vec3 >::getPos() const { return m_pos; }
	
	template <typename Float, typename Vec3>
	bool Point< Float, Vec3 >::equal( const Point< Float, Vec3 >& other, const float& epsilon ) const
	{
		return 	glm::distance2( *m_color, *other.m_color ) < epsilon && glm::distance2( *m_pos, *other.m_pos ) < epsilon;
	}
	
	template <typename Float, typename Vec3>
	Point< Float, Vec3 > Point< Float, Vec3 >::multiply( const Float& multiplier ) const
	{
		return Point< Float, Vec3 >( *m_color * multiplier, *m_pos * multiplier );
	}
	
	template < typename Float, typename Vec3 >
	Point< Float, Vec3 > operator+( const Point< Float, Vec3 >& left, const Point< Float, Vec3 >& right )
	{
		return Point< Float, Vec3 >( *left.m_color + *right.m_color, *left.m_pos + *right.m_pos );
	}
	
	template < typename Float, typename Vec3 >
	Point< Float, Vec3 > operator+( Point< Float, Vec3 >&& left , const Point< Float, Vec3 >& right )
	{
		return Point< Float, Vec3 >( *left.m_color + *right.m_color, *left.m_pos + *right.m_pos );
	}
	
	template < typename Float, typename Vec3 >
	Point< Float, Vec3 > operator+( const Point< Float, Vec3 >& left, Point< Float, Vec3 >&& right )
	{
		return Point< Float, Vec3 >( *left.m_color + *right.m_color, *left.m_pos + *right.m_pos );
	}
	
	template < typename Float, typename Vec3 >
	Point< Float, Vec3 > operator+( Point< Float, Vec3 >&& left, Point< Float, Vec3 >&& right )
	{
		return Point< Float, Vec3 >( *left.m_color + *right.m_color, *left.m_pos + *right.m_pos );
	}
	
	template < typename Float, typename Vec3 >
	ostream& operator<< ( ostream &out, const Point< Float, Vec3 > &point )
	{
		Vec3 pos = *point.m_pos;
		Vec3 color = *point.m_color;
		
		out << "Point:" << endl
			<< "color = " << glm::to_string( color ) << endl
			<< "pos = " << glm::to_string( pos ) << endl;
			
		return out;
	}
	
	template < typename Float, typename Vec3 >
	size_t Point< Float, Vec3 >::serialize( byte** serialization ) const
	{
		size_t sizeOfVec3 = sizeof( Vec3 );
		size_t sizeOfPoint = 2 * sizeOfVec3;
		
		*serialization = new byte[ sizeOfPoint ];
		memcpy( *serialization, &( *m_color ) ,sizeOfVec3 );
		memcpy( *serialization + sizeOfVec3, &( *m_pos ), sizeOfVec3 );
		
		return sizeOfPoint;
	}
	
	//===========
	// Type sugar
	//===========
	
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