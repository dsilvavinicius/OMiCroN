#ifndef HEAP_POINT_READER_H
#define HEAP_POINT_READER_H

#include "PointReader.h"
#include "PointSorter.h"
#include "OctreeDimensions.h"
#include <queue>

using namespace model;

namespace util
{
	/** PointReader which maintains a min heap to return sorted points as early as possible. */
	template< typename Morton >
	class HeapPointReader
	: public PointReader
	{
	public:
		using Sorter = PointSorter< Morton >;
		using Dim = OctreeDimensions< Morton >;
		
		typedef struct Comparator
		{
			Comparator() {}
			
			Comparator( const Dim& dim )
			: m_dim( dim ) {}
			
			bool operator()( const Point& p0, const Point& p1 ) const
			{
				return m_dim( p1, p0 );
			}
			
			Dim m_dim;
		} Comparator;
		
		using Heap = priority_queue< Point, deque< Point, TbbAllocator< Point > >, Comparator >;
		using HeapPtr = unique_ptr< Heap >;
		
		HeapPointReader( const string& filename, uint leafLvl );
		
		void read( const function< void( const Point& ) >& onPointDone ) override;
	
		const Dim& dimensions() const { return m_dim; }
		
	private:
		HeapPtr m_heap;
		Dim m_dim;
	};
	
	template< typename Morton >
	HeapPointReader< Morton >::HeapPointReader( const string& filename, uint leafLvl )
	{
		Sorter sorter( filename, leafLvl );
		PointSet< Morton > points = sorter.points();
		m_dim = points.m_dim;
		
		m_heap = HeapPtr( new Heap( Comparator( points.m_dim ), std::move( *points.m_points ) ) );
		
		// DEBUG
		{
			cout << "HEAP CONSTRUCTED." << endl << endl;
		}
	}

	template< typename Morton >
	void HeapPointReader< Morton >::read( const function< void( const Point& ) >& onPointDone )
	{
		// DEBUG
		{
			cout << "START READING." << endl << endl;
		}
		
		while( !m_heap->empty() )
		{
			// DEBUG
// 			{
// 				cout << "s: " << m_heap->size() << endl;
// 			}
			
			onPointDone( m_heap->top() );
			
			m_heap->pop();
		}
	}
}

#endif