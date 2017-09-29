#ifndef IN_MEM_POINT_READER_H
#define IN_MEM_POINT_READER_H

#include "PointReader.h"
#include <OctreeDimensions.h>
#include <deque>

namespace util
{
	template< typename Morton >
	class InMemPointReader
	: public PointReader
	{
	public:
		using PointDeque = deque< Point, TbbAllocator< Point > >;
		using PointDequePtr = shared_ptr< PointDeque >;
		using Dim = OctreeDimensions< Morton >;
		
		InMemPointReader( PointDequePtr points, const Dim& dim );
		void read( const function< void( const Point& ) >& onPointDone ) override;
	
		const Dim& dimensions() const { return m_dim; }
		
	private:
		PointDequePtr m_points;
		Dim m_dim;
	};
	
	template< typename Morton >
	inline InMemPointReader< Morton >::InMemPointReader( PointDequePtr points, const Dim& dim )
	: m_points( points ),
	m_dim( dim )
	{}
	
	template< typename Morton >
	inline void InMemPointReader< Morton >::read( const function< void( const Point& ) >& onPointDone )
	{
		while( !m_points->empty() )
		{
			onPointDone( m_points->front() );
			m_points->pop_front();
		}
		
// 		for( const Point& p : *m_points )
// 		{
// 			onPointDone( p );
// 		}
		
// 		ulong readBeforeShrink = m_points->size() / 10;
// 		
// 		while( !m_points->empty() )
// 		{
// 			typename PointVector::iterator it = m_points->begin();
// 			
// 			for( int i = 0; i < readBeforeShrink && it != m_points->end(); ++i, it++ )
// 			{
// 				onPointDone( *it );
// 			}
// 			
// 			m_points->erase( m_points->begin(), it );
// 		}
	}
}

#endif