#ifndef EXTENDED_POINT_H
#define EXTENDED_POINT_H

#include "Point.h"

using namespace std;

namespace model
{
	/** Point with normal. */
	class ExtendedPoint : public Point
	{
	public:
		ExtendedPoint();
		ExtendedPoint( const ExtendedPoint& other );
		ExtendedPoint& operator=( const ExtendedPoint& other );
		ExtendedPoint( const Vec3& color, const Vec3& normal, const Vec3& pos );
		ExtendedPoint( ExtendedPoint&& other );
		ExtendedPoint& operator=( ExtendedPoint&& other);
		ExtendedPoint( byte* serialization );
		ExtendedPoint( byte* serialization, byte*& pastRead );
		
		~ExtendedPoint();
		
		shared_ptr< Vec3 > getNormal();
		
		const shared_ptr< const Vec3 > getNormal() const;
		
		// Comparison operators.
		bool equal( const ExtendedPoint& other, const float& epsilon ) const;
		
		// Arithmetic operators.
		ExtendedPoint multiply( const Float& multiplier ) const;
		
		friend ExtendedPoint operator+( const ExtendedPoint& left, const ExtendedPoint& right );
		
		friend ExtendedPoint operator+( ExtendedPoint&& left , const ExtendedPoint& right );
		
		friend ExtendedPoint operator+( const ExtendedPoint& left, ExtendedPoint&& right );
		
		friend ExtendedPoint operator+( ExtendedPoint&& left, ExtendedPoint&& right );
		
		friend ostream& operator<<( ostream &out, const ExtendedPoint& point );
		
		size_t serialize( byte** serialization ) const;
		
	protected:
		shared_ptr< Vec3 > m_normal;
	};
	
	//===========
	// Type sugar
	//===========
	
	/** ExtendedPoint smart pointer. */
	using ExtendedPointPtr = shared_ptr< ExtendedPoint >;
	
	/** Vector of ExtendedPoints. */
	using ExtendedPointVector = vector< ExtendedPointPtr >;
	
	/** Ptr for Vector of ExtendedPoints. */
	using ExtendedPointVectorPtr = shared_ptr< ExtendedPointVector >;
}

#endif