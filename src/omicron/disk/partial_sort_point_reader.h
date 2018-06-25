#ifndef PARTIAL_POINT_SORT_READER_H
#define PARTIAL_POINT_SORT_READER_H

#include "omicron/disk/point_reader.h"
#include "omicron/disk/point_sorter.h"
#include "omicron/hierarchy/reconstruction_params.h"

namespace omicron::disk
{
	template< typename Morton >
	class PartialSortPointReader
	: public PointReader
	{
	public:
		using Dim = OctreeDimensions< Morton >;
		
		PartialSortPointReader( const string& filename, uint leafLvl );
		
		void read( const function< void( const Point& ) >& onPointDone );
		
		const Dim& dimensions() const { return m_pointSet.m_dim; }
	
	private:
		using Sorter = PointSorter< Morton >;
		using PointSet = disk::PointSet< Morton >;
		
		PointSet m_pointSet;
		ulong m_sortedPerIter;
	};
	
	template< typename Morton >
	PartialSortPointReader< Morton >::PartialSortPointReader( const string& filename, uint leafLvl )
	: PointReader()
	{
		PointSorter< Morton > sorter( filename, leafLvl );
		m_pointSet = sorter.points();
		
		m_inputTime = sorter.inputTime();
		
		typename PointSet::PointDeque& points = *m_pointSet.m_points;
		m_sortedPerIter = points.size() / SORTING_SEGMENTS;
		
		auto now = Profiler::now( "Initial partial sort" );
		
		// The first partial sort is done now so points are ready to be loaded at rendering starting time.
		std::partial_sort( points.begin(), points.begin() + m_sortedPerIter, points.end(), m_pointSet.m_dim );
		
		m_initTime = Profiler::elapsedTime( now, "Initial partial sort" );
	}

	template< typename Morton >
	void PartialSortPointReader< Morton >::read( const function< void( const Point& ) >& onPointDone )
	{
		auto now = Profiler::now( "Partial sorter point reading" );
		
		typename PointSet::PointDeque& points = *m_pointSet.m_points;
		typename PointSet::PointDeque::iterator endIter = points.begin() + m_sortedPerIter;
		
		while( true )
		{
			while( points.begin() != endIter )
			{
				onPointDone( points.front() );
				points.pop_front();
			}
			
			if( points.empty() ) break;
			
			endIter = ( points.size() <= m_sortedPerIter ) ? points.end() : points.begin() + m_sortedPerIter;
			std::partial_sort( points.begin(), endIter, points.end(), m_pointSet.m_dim );
		}
		
		m_readTime = Profiler::elapsedTime( now, "Partial sorter point reading" );
	}
	
}

#endif
