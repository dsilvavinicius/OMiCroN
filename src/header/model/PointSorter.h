#ifndef POINT_SORTER_H
#define POINT_SORTER_H

#include <jsoncpp/json/json.h>
#include <fstream>
#include "MortonCode.h"
#include "PlyPointReader.h"
#include "PlyPointWritter.h"
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
		using Writter = PlyPointWritter< P >;
		using OctreeDim = model::OctreeDimensions< M, P >;
		
		PointSorter( const string& input, const string& outFilename, uint leafLvl );
		~PointSorter();
		Json::Value sort();
		OctreeDim& comp() { return m_comp; }
		
	private:
		M calcMorton( const P& point );
		void write( const Point& p );
		void write( const ExtendedPoint& p );
		
		Reader m_reader;
		Writter m_writter;
		
		P* m_points;
		long m_nPoints;
		
		OctreeDim m_comp;
	};
	
	template< typename M, typename P >
	PointSorter< M, P >::PointSorter( const string& input, const string& outFilename, uint leafLvl )
	: m_reader( input ),
	m_writter( m_reader, outFilename, m_reader.getNumPoints() )
	{
		cout << "Setup sorting of " << input << endl << endl;
		
		m_nPoints = m_reader.getNumPoints();
		m_points = ( P* ) malloc( sizeof( P ) * m_nPoints );
		
		Float negInf = -numeric_limits< Float >::max();
		Float posInf = numeric_limits< Float >::max();
		Vec3 origin = Vec3( posInf, posInf, posInf );
		Vec3 maxCoords( negInf, negInf, negInf );
		
		cout << "Reading " << m_nPoints << " points." << endl << endl;
		auto start = Profiler::now();
		
		long i = 0;
		m_reader.read(
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
		
		// Normalizing model.
		float scale = 1.f / std::max( std::max( octreeSize.x(), octreeSize.y() ), octreeSize.z() );
		
		#pragma omp parallel for
		for( ulong i = 0; i < m_nPoints; ++i )
		{
			Vec3& pos = m_points[ i ].getPos();
			pos = ( pos - origin ) * scale;
		}
		
		m_comp.init( Vec3( 0.f, 0.f, 0.f ), octreeSize * scale, leafLvl );
		
		// Debug
		{
			cout << "Scale: " << scale << endl << "Octree dim: " <<  m_comp << endl;
		}
	}
	
	template< typename M, typename P >
	PointSorter< M, P >::~ PointSorter()
	{
		free( m_points );
	}
	
	template< typename M, typename P >
	Json::Value PointSorter< M, P >::sort()
	{
		const string& outFilename = m_writter.filename();
		cout << "Start sorting to " << outFilename << endl << endl;
		
		// Write the octree file.
		string dbFilename = outFilename.substr( 0, outFilename.find_last_of( '.' ) );
		string octreeFilename = dbFilename;
		dbFilename.append( ".db" );
		octreeFilename.append( ".oct" );
		
		Json::Value octreeJson;
		octreeJson[ "points" ] = outFilename;
		octreeJson[ "database" ] = dbFilename;
		octreeJson[ "size" ][ "x" ] = m_comp.m_size.x();
		octreeJson[ "size" ][ "y" ] = m_comp.m_size.y();
		octreeJson[ "size" ][ "z" ] = m_comp.m_size.z();
		octreeJson[ "depth" ] = m_comp.m_nodeLvl;
		
		ofstream octreeFile( octreeFilename, ofstream::out );
		octreeFile << octreeJson << endl;
		
		auto start = Profiler::now();
		
		// Sort points.
		std::sort( m_points, m_points + m_nPoints, m_comp );
		
		cout << "Sorting time (ms): " << Profiler::elapsedTime( start ) << endl << endl;
		
		cout << "Start writing. " << endl << endl;
		
		start = Profiler::now();
		
		// Write output point file.
		for( long i = 0; i < m_nPoints; ++i )
		{
			m_writter.write( m_points[ i ] );
		}
		
		cout << "Writing time (ms): " << Profiler::elapsedTime( start ) << endl << endl;
		
		return octreeJson;
	}
}

#endif