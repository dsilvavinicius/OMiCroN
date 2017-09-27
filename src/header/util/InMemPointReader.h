#ifndef IN_MEM_POINT_READER_H
#define IN_MEM_POINT_READER_H

#include "PointReader.h"

namespace util
{
	class InMemPointReader
	: public PointReader
	{
	public:
		using PointVector = vector< Point, TbbAllocator< Point > >;
		using PointVectorPtr = shared_ptr< PointVector >;
		
		InMemPointReader( PointVectorPtr points );
		void read( const function< void( const Point& ) >& onPointDone ) override;
	
	private:
		PointVectorPtr m_points;
		PointVector::iterator m_it;
	};
	
	inline InMemPointReader::InMemPointReader( PointVectorPtr points )
	: m_points( points ),
	m_it( m_points->begin() )
	{}
	
	inline void InMemPointReader::read( const function< void( const Point& ) >& onPointDone )
	{
		for( const Point& p : ( *m_points ) )
		{
			onPointDone( p );
		}
	}
}

#endif