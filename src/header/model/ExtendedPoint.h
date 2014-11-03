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
	template < typename Float, typename Vec3 >
	class ExtendedPoint : public Point< Float, Vec3 >
	{
		ExtendedPoint();
		ExtendedPoint( const Vec3& color, const Vec3& normal, const Vec3& pos );
		
		shared_ptr< Vec3 > getNormal();
		
		// Comparison operators.
		bool equal(const ExtendedPoint< Float, Vec3 >& other);
		
		// Arithmetic operators.
		ExtendedPoint< Float, Vec3 > multiply( const Float& multiplier ) const;
		
		template < typename F, typename V >
		friend ExtendedPoint< F, V > operator+( const ExtendedPoint< F, V >& left, const ExtendedPoint< F, V >& right );
		
		template < typename F, typename V >
		friend ExtendedPoint< F, V > operator+( ExtendedPoint< F, V >&& left , const ExtendedPoint< F, V >& right );
		
		template < typename F, typename V >
		friend ExtendedPoint< F, V > operator+( const ExtendedPoint< F, V >& left, ExtendedPoint< F, V >&& right );
		
		template < typename F, typename V >
		friend ExtendedPoint< F, V > operator+( ExtendedPoint< F, V >&& left, ExtendedPoint< F, V >&& right );
		
		template < typename F, typename V >
		friend ostream& operator<< ( ostream &out, ExtendedPoint< F, V > &point );
		
	private:
		shared_ptr< Vec3 > m_normal;
	};
	
	template <typename Float, typename Vec3>
	ExtendedPoint< Float, Vec3 >::ExtendedPoint()
	{
		Point< Float, Vec3 >::Point();
		m_normal = make_shared< Vec3 >( Vec3( 0, 0, 0 ) );
	}
	
	template <typename Float, typename Vec3>
	ExtendedPoint< Float, Vec3 >::ExtendedPoint(const Vec3& color, const Vec3& normal, const Vec3& pos)
	{
		Point< Float, Vec3 >::Point( color, pos );
		m_normal = make_shared< Vec3 >( normal );
	}
	
	template <typename Float, typename Vec3>
	shared_ptr< Vec3 > ExtendedPoint< Float, Vec3 >::getNormal() { return m_normal; }
	
	template <typename Float, typename Vec3>
	bool ExtendedPoint< Float, Vec3 >::equal( const ExtendedPoint< Float, Vec3 >& other )
	{
		return Point< Float, Vec3 >::equal() && glm::all( glm::equal( *m_normal, *other.m_normal ) );
	}
	
	template <typename Float, typename Vec3>
	ExtendedPoint< Float, Vec3 > ExtendedPoint< Float, Vec3 >::multiply( const Float& multiplier ) const
	{
		return ExtendedPoint< Float, Vec3 >(*m_color * multiplier, *m_normal * multiplier, *m_pos * multiplier);
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
	
	template < typename Float, typename Vec3 >
	ostream& operator<<( ostream &out, ExtendedPoint< Float, Vec3 > &point )
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