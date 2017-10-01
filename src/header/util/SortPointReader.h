#ifndef SORT_POINT_READER_H
#define SORT_POINT_READER_H

#include "PointReader.h"
#include "PointSorter.h"
#include <deque>

namespace util
{
	template< typename Morton >
	class SortPointReader
	: public PointReader
	{
	public:
		using PointDeque = deque< Point, TbbAllocator< Point > >;
		using PointDequePtr = shared_ptr< PointDeque >;
		using Dim = OctreeDimensions< Morton >;
		
		SortPointReader( const string& filename, uint leafLvl );
		void read( const function< void( const Point& ) >& onPointDone ) override;
	
		const Dim& dimensions() const { return m_pointSet.m_dim; }
		
	private:
		using Sorter = PointSorter< Morton >;
		using PointSet = model::PointSet< Morton >;
		
		PointSet m_pointSet;
	};
	
	template< typename Morton >
	inline SortPointReader< Morton >::SortPointReader( const string& filename, uint leafLvl )
	{
		Sorter sorter( filename, leafLvl );
		PointSet points = sorter.sort();
		
		m_inputTime = sorter.inputTime();
		m_initTime = sorter.sortTime();
		m_outputTime = sorter.outputTime();
	}
	
	template< typename Morton >
	inline void SortPointReader< Morton >::read( const function< void( const Point& ) >& onPointDone )
	{
		typename PointSet::PointDeque& points = *m_pointSet.m_points;
		
		while( !points.empty() )
		{
			onPointDone( points.front() );
			points.pop_front();
		}
	}
}

#endif