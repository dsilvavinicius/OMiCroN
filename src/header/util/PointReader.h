#ifndef POINT_READER_H
#define POINT_READER_H

#include "Point.h"
#include <OctreeDimensions.h>

using namespace model;

namespace util
{
	// Interface for reading points.
	class PointReader
	{
	public: 
		virtual void read( const function< void( const Point& ) >& onPointDone ) = 0;
	};
}

#endif