#ifndef POINT_H
#define POINT_H

#include <memory>
#include <vector>
#include <iostream>
#include "BasicTypes.h"
#include "ManagedAllocator.h"

using namespace std;

namespace model
{
	/** Point representation. */
	class Point
	{
	public:
		Point();
		Point( const Point& other );
		Point& operator=( const Point& other );
		Point( const Vec3& normal, const Vec3& pos );
		Point( Point&& other );
		Point& operator=( Point&& other);
		
		/** Deserialization constructor.
		 *	@param serialization must be acquired by the serialize() method.*/
		Point( byte* serialization );
		
		/** Deserialization constructor that returns a pointer past read data.
		 *	@param serialization must be acquired by the serialize() method.
		 *	@param pastReadPtr returns a pointer to data past the reading. */
		Point( byte* serialization, byte*& pastRead );
		
		void* operator new( size_t size );
		void* operator new[]( size_t size );
		void operator delete( void* p );
		void operator delete[]( void* p );
		
		Vec3& getNormal();
		Vec3& getPos();
		
		const Vec3& getNormal() const;
		const Vec3& getPos() const;
		
		// Comparison operators.
		bool equal( const Point& other, const Float& epsilon ) const;
		
		// Arithmetic operators.
		Point multiply( const Float& multiplier ) const;
		
		friend Point operator+( const Point& left, const Point& right );
		
		friend Point operator+( Point&& left , const Point& right );
		
		friend Point operator+( const Point& left, Point&& right );
		
		friend Point operator+( Point&& left, Point&& right );
		
		friend ostream& operator<< ( ostream &out, const Point& point );
		
		/** Serializes the point. The caller is responsible to delete the memory.
		 *  @param serialization is a pointer that will have the byte array at method return.
		 *  @returns the size of the byte array. */
		size_t serialize( byte** serialization ) const;
		
	protected:
		Vec3 m_normal;
		Vec3 m_pos;
	};
	
	//===========
	// Type sugar
	//===========
	
	/** Point smart pointer. */
	using PointPtr = shared_ptr< Point >;
	
	/** Vector of Points. */
	using PointVector = vector< PointPtr, ManagedAllocator< PointPtr > >;
	
	/** Ptr for Vector of Points. */
	using PointVectorPtr = shared_ptr< PointVector >;
}

#endif