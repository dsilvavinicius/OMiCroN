#ifndef SURFEL_H
#define SURFEL_H

#include <iostream>
#include <Eigen/Dense>

using namespace std;

struct Surfel
{
	Surfel() { }

	Surfel(Eigen::Vector3f c_, Eigen::Vector3f u_, Eigen::Vector3f v_ )
		: c(c_), u(u_), v(v_) { }

	friend ostream& operator<<( ostream& out, const Surfel& surfel );
		
	Eigen::Vector3f c,      // Position of the ellipse center point.
					u, v;   // Ellipse major and minor axis.
};

inline ostream& operator<<( ostream& out, const Surfel& surfel )
{
	out << "{" << endl << "Center: " << endl << surfel.c << endl << "Tangent u: " << endl << surfel.u << endl
		<< " Tangent v: " << endl << surfel.v << endl << "}";
	return out;
}
	
#endif