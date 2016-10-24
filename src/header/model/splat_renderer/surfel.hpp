#ifndef SURFEL_H
#define SURFEL_H

	struct Surfel
	{
		Surfel() { }

		Surfel(Eigen::Vector3f c_, Eigen::Vector3f u_, Eigen::Vector3f v_ )
			: c(c_), u(u_), v(v_) { }

	//     Surfel( Surfel&& other )
	// 	: c( other.c ),
	// 	u( other.u ),
	// 	v( other.v )
	// 	{
	// 	}
	// 	
	// 	Surfel& operator=( Surfel&& other )
	// 	{
	// 		c = other.c;
	// 		u = other.u;
	// 		v = other.v;
	// 		
	// 		return *this;
	// 	}
			
		Eigen::Vector3f c,      // Position of the ellipse center point.
						u, v;   // Ellipse major and minor axis.
	};

#endif