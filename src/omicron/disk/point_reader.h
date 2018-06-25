#ifndef POINT_READER_H
#define POINT_READER_H

#include "omicron/basic/point.h"
#include "omicron/hierarchy/octree_dimensions.h"

using namespace omicron;

namespace omicron::disk
{
	// Interface for reading points.
	class PointReader
	{
	public: 
		PointReader()
		: m_inputTime( 0u ),
		m_initTime( 0u ),
		m_readTime( 0u )
		{};
		
		/** Read all points using the callback for all read point. */
		virtual void read( const function< void( const Point& ) >& onPointDone ) = 0;
	
		/** @returns the time needed to read points from input file (in ms). */
		uint inputTime() const { return m_inputTime; }
		
		/** @returns the time needed to init the reader (a partial or full sort for example) (in ms). */
		uint initTime() const { return m_initTime; }
		
		/** @returns the time needed to read the points after init (in ms). */
		uint readTime() const { return m_readTime; }
	
	protected:
		uint m_inputTime; // Time needed to read points from input file (in ms).
		uint m_initTime; // Time needed to init the reader (a partial or full sort for example) (in ms).
		uint m_readTime; // Time needed to output points to output file (if needed) (in ms).
	};
}

#endif
