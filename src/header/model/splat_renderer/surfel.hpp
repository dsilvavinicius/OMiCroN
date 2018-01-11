#ifndef SURFEL_H
#define SURFEL_H

#include <iostream>
#include <fstream>
#include <Eigen/Dense>
#include "Point.h"
#include "ReconstructionParams.h"

using namespace std;
using namespace model;

struct Surfel
{
	Surfel() { }

	Surfel(Eigen::Vector3f c_, Eigen::Vector3f u_, Eigen::Vector3f v_ )
		: c(c_), u(u_), v(v_) { }
	
	
	Surfel( const Point& p );
	
	void multiplyTangents( const Vector2f& multipliers );
	
	/** Ctor to init from stream.
	 * @param input is expected to be binary and writen with persist() */
	Surfel( ifstream& input );
	
	bool operator==( const Surfel& other ) const;
	
	string toString() const;
	
	/** Binary persistence. Structure: | c | u | v |. */
	void persist( ostream& out ) const;
	
	friend ostream& operator<<( ostream& out, const Surfel& surfel );
	
	Eigen::Vector3f c,      // Position of the ellipse center point.
					u, v;   // Ellipse major and minor axis.
};

inline Surfel::Surfel( const Point& p )
{
	const Vec3& pos = p.getPos();
	const Vec3& normal = p.getNormal();
	
	// This can lead to a division by 0.
	float epsilon = 1e-10;
	float planeMinusD = normal.x() * pos.x() + normal.y() * pos.y() + normal.z() * pos.z();
	
	Vec3 pointOnPlane;
	if( fabs( normal.x() ) > epsilon )
	{
		pointOnPlane = Vec3( planeMinusD / normal.x(), 0.f, 0.f );
	}
	else if( fabs( normal.y() ) > epsilon )
	{
		pointOnPlane = Vec3( 0.f, planeMinusD / normal.y(), 0.f );
	}
	else if( fabs( normal.z() ) > epsilon )
	{
		pointOnPlane = Vec3( 0.f, 0.f, planeMinusD / normal.z() );
	}
// 	else
// 	{
// // 							cout << "Read point has zero vector normal." << endl << endl;
// // 		return;
// 	}
	
	u = pointOnPlane - pos;
	u.normalize();
	v = normal.cross( u );

	c = pos;
	u *= LEAF_SURFEL_TANGENT_SIZE_X;
	v *= LEAF_SURFEL_TANGENT_SIZE_Y;
}

inline Surfel::Surfel( ifstream& input )
{
	float x, y, z;
	
	input.read( reinterpret_cast< char* >( &x ), sizeof( float ) );
	input.read( reinterpret_cast< char* >( &y ), sizeof( float ) );
	input.read( reinterpret_cast< char* >( &z ), sizeof( float ) );
	
	c = Eigen::Vector3f( x, y, z );
	
	input.read( reinterpret_cast< char* >( &x ), sizeof( float ) );
	input.read( reinterpret_cast< char* >( &y ), sizeof( float ) );
	input.read( reinterpret_cast< char* >( &z ), sizeof( float ) );
	
	u = Eigen::Vector3f( x, y, z );
	
	input.read( reinterpret_cast< char* >( &x ), sizeof( float ) );
	input.read( reinterpret_cast< char* >( &y ), sizeof( float ) );
	input.read( reinterpret_cast< char* >( &z ), sizeof( float ) );
	
	v = Eigen::Vector3f( x, y, z );
}

inline void Surfel::multiplyTangents( const Vector2f& multipliers )
{
	u *= multipliers.x();
	v *= multipliers.y();
}

inline bool Surfel::operator==( const Surfel& other ) const
{
	return c.isApprox( other.c ) && u.isApprox( other.u ) && v.isApprox( other.v );
}

inline string Surfel::toString() const
{
	stringstream ss;
	ss << "{" << endl << "Center: " << endl << c << endl << "Tangent u: " << endl << u << endl << " Tangent v: " << endl << v << endl << "}";
	
	return ss.str();
}

inline void Surfel::persist( ostream& out ) const
{
	out.write( reinterpret_cast< const char* >( &c.x() ), sizeof( float ) );
	out.write( reinterpret_cast< const char* >( &c.y() ), sizeof( float ) );
	out.write( reinterpret_cast< const char* >( &c.z() ), sizeof( float ) );
	
	out.write( reinterpret_cast< const char* >( &u.x() ), sizeof( float ) );
	out.write( reinterpret_cast< const char* >( &u.y() ), sizeof( float ) );
	out.write( reinterpret_cast< const char* >( &u.z() ), sizeof( float ) );
	
	out.write( reinterpret_cast< const char* >( &v.x() ), sizeof( float ) );
	out.write( reinterpret_cast< const char* >( &v.y() ), sizeof( float ) );
	out.write( reinterpret_cast< const char* >( &v.z() ), sizeof( float ) );
}

inline ostream& operator<<( ostream& out, const Surfel& surfel )
{
	out << surfel.toString();
	
	return out;
}
	
#endif