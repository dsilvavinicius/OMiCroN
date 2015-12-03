#ifndef POINT_SORTER_H
#define POINT_SORTER_H

#include "MortonCode.h"
#include "PlyPointReader.h"
#include "OctreeDimensions.h"
#include "Profiler.h"

using namespace std;
using namespace util;

namespace model
{
	/** Sorts a point .ply file in z order.
	 * @param M is the MortonCode type.
	 * @param P is the Point type. */
	template< typename M, typename P >
	class PointSorter
	{
	public:
		using Reader = PlyPointReader< P >;
		using OctreeDim = model::OctreeDimensions< M, P >;
		
		PointSorter( const string& input, uint leafLvl );
		~PointSorter();
		void sort( const string& outFilename );
		OctreeDim& comp() { return m_comp; }
		
	private:
		M calcMorton( const P& point );
		void write( const Point& p );
		void write( const ExtendedPoint& p );
		
		Reader* m_reader;
		p_ply m_output;
		
		P* m_points;
		long m_nPoints;
		
		OctreeDim m_comp;
	};
	
	template< typename M, typename P >
	PointSorter< M, P >::PointSorter( const string& input, uint leafLvl )
	{
		m_reader = new Reader( input );
		m_nPoints = m_reader->getNumPoints();
		m_points = ( P* ) malloc( sizeof( P ) * m_nPoints );
		
		Float negInf = -numeric_limits< Float >::max();
		Float posInf = numeric_limits< Float >::max();
		Vec3 origin = Vec3( posInf, posInf, posInf );
		Vec3 maxCoords( negInf, negInf, negInf );
		
		cout << "Reading points." << endl << endl;
		auto start = Profiler::now();
		
		long i = 0;
		m_reader->read( Reader::SINGLE,
			[ & ]( const P& p )
			{
				m_points[ i++ ] = p;
				Vec3 pos = p.getPos();
				for( int i = 0; i < 3; ++i )
				{
					origin[ i ] = glm::min( origin[ i ], pos[ i ] );
					maxCoords[ i ] = glm::max( maxCoords[ i ], pos[ i ] );
				}
			}
		);
		
		cout << "Reading time (ms): " << Profiler::elapsedTime( start ) << endl << endl;
		
		Vec3 octreeSize = maxCoords - origin;
		m_comp.init( origin, octreeSize, leafLvl );
	}
	
	template< typename M, typename P >
	PointSorter< M, P >::~ PointSorter()
	{
		free( m_points );
		delete m_reader;
	}
	
	template< typename M, typename P >
	void PointSorter< M, P >::sort( const string& outFilename )
	{
		cout << "In-memory sort. " << endl << endl;
		auto start = Profiler::now();
		
		std::sort( m_points, m_points + m_nPoints, m_comp );
		
		cout << "Sorting time (ms): " << Profiler::elapsedTime( start ) << endl << endl;
		
		cout << "Writing. " << endl << endl;
		
		start = Profiler::now();
		// Write output file
		m_output = m_reader->copyHeader( outFilename );
		for( long i = 0; i < m_nPoints; ++i )
		{
			write( m_points[ i ] );
		}
		
		cout << "Writing time (ms): " << Profiler::elapsedTime( start ) << endl << endl;
		
		ply_close( m_output );
	}
	
	template< typename M, typename P >
	inline void PointSorter< M, P >::write( const Point& p )
	{
		Vec3 pos = p.getPos();
		ply_write( m_output, pos.x );
		ply_write( m_output, pos.y );
		ply_write( m_output, pos.z );
		
		Vec3 normal = p.getColor();
		ply_write( m_output, normal.x );
		ply_write( m_output, normal.y );
		ply_write( m_output, normal.z );
	}
	
	template< typename M, typename P >
	inline void PointSorter< M, P >::write( const ExtendedPoint& p )
	{
		Vec3 pos = p.getPos();
		ply_write( m_output, pos.x );
		ply_write( m_output, pos.y );
		ply_write( m_output, pos.z );
		
		Vec3 normal = p.getNormal();
		ply_write( m_output, normal.x );
		ply_write( m_output, normal.y );
		ply_write( m_output, normal.z );
		
		Vec3 color = p.getColor();
		ply_write( m_output, color.x );
		ply_write( m_output, color.y );
		ply_write( m_output, color.z );
	}
}

#endif