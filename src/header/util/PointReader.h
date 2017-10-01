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
		PointReader()
		: m_inputTime( 0u ),
		m_initTime( 0u ),
		m_outputTime( 0u )
		{};
		
		/** Read all points using the callback for all read point. */
		virtual void read( const function< void( const Point& ) >& onPointDone ) = 0;
	
		/** @returns the time needed to read points from input file (in ms). */
		uint inputTime() const { return m_inputTime; }
		
		/** @returns the time needed to init the reader (a partial or full sort for example) (in ms). */
		uint initTime() const { return m_initTime; }
		
		/** @returns the time needed to output points to output file (if needed) (in ms). */
		uint outputTime() const { return m_outputTime; }
	
	protected:
		uint m_inputTime; // Time needed to read points from input file (in ms).
		uint m_initTime; // Time needed to init the reader (a partial or full sort for example) (in ms).
		uint m_outputTime; // Time needed to output points to output file (if needed) (in ms).
	};
}

#endif