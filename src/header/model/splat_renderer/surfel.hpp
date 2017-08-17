#ifndef SURFEL_H
#define SURFEL_H

#include <iostream>
#include <fstream>
#include <Eigen/Dense>

using namespace std;

struct Surfel
{
	Surfel() { }

	Surfel(Eigen::Vector3f c_, Eigen::Vector3f u_, Eigen::Vector3f v_ )
		: c(c_), u(u_), v(v_) { }
	
	/** Ctor to init from stream.
	 * @param input is expected to be binary and writen with persist() */
	Surfel( ifstream& input );
	
	string toString() const;
	
	/** Binary persistence. Structure: | c | u | v |. */
	void persist( ostream& out ) const;
	
	friend ostream& operator<<( ostream& out, const Surfel& surfel );
	
	Eigen::Vector3f c,      // Position of the ellipse center point.
					u, v;   // Ellipse major and minor axis.
};

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