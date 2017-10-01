#ifndef POINT_SORTER_H
#define POINT_SORTER_H

#include <jsoncpp/json/json.h>
#include <fstream>
#include "MortonCode.h"
#include "PlyPointReader.h"
#include "PlyPointWritter.h"
#include "OctreeDimensions.h"
#include "Profiler.h"
#include "global_malloc.h"
#include "PointSet.h"

using namespace std;
using namespace util;

namespace model
{
	/** Sorts a point .ply file in z order.
	 * @param M is the MortonCode type. */
	template< typename M >
	class PointSorter
	{
	public:
		using Reader = PlyPointReader;
		using Writter = PlyPointWritter;
		using OctreeDim = model::OctreeDimensions< M >;
		using SorterPointSet = model::PointSet< M >;
		
		PointSorter( const string& input, uint leafLvl );
		PointSorter( const string& input, const OctreeDim& dim );
		
		SorterPointSet sort();
		Json::Value sortToFile( const string& outFilename );
		OctreeDim& comp() { return m_comp; }
		
		/** @returns the points. If sort() was called before, the points are sorted. They will be in an unknown order otherwise. */
		PointSet< M > points() { return PointSet< M >( m_points, m_comp ); }
		
		/** @returns the time to read points (in ms). */
		uint inputTime() const { return m_inputTime; }
		
		/** @returns the time to sort points (in ms) */
		uint sortTime() const { return m_sortTime; }
		
		/** @returns the time to output points (in ms) */
		uint outputTime() const { return m_outputTime; }
		
	private:
		M calcMorton( const Point& point );
		void write( const Point& p );
		
		Reader m_reader;
		
		typename SorterPointSet::PointDequePtr m_points;
		
		OctreeDim m_comp;
		
		uint m_inputTime; // Time to read points (in ms).
		uint m_sortTime; // Time to sort points (in ms).
		uint m_outputTime; // Time to output points (in ms).
	};
	
	template< typename M >
	PointSorter< M >::PointSorter( const string& input, uint leafLvl )
	: m_reader( input ),
	m_sortTime( 0u ),
	m_outputTime( 0u )
	{
		cout << "Setup sorting of " << input << endl << endl;
		
		m_points = typename SorterPointSet::PointDequePtr( new typename SorterPointSet::PointDeque( m_reader.getNumPoints() ) );
		
		Float negInf = -numeric_limits< Float >::max();
		Float posInf = numeric_limits< Float >::max();
		Vec3 origin = Vec3( posInf, posInf, posInf );
		Vec3 maxCoords( negInf, negInf, negInf );
		
		cout << "Reading " << m_points->size() << " points." << endl << endl;
		auto start = Profiler::now();
		
		long i = 0;
		m_reader.read(
			[ & ]( const Point& p )
			{
				( *m_points )[ i++ ] = p;
				Vec3 pos = p.getPos();
				for( int i = 0; i < 3; ++i )
				{
					origin[ i ] = std::min( origin[ i ], pos[ i ] );
					maxCoords[ i ] = std::max( maxCoords[ i ], pos[ i ] );
				}
			}
		);
		
		m_inputTime = Profiler::elapsedTime( start );
		
		cout << "Reading time (ms): " << m_inputTime << endl << endl;
		
		start = Profiler::now();
		
		Vec3 octreeSize = maxCoords - origin;
		
		// Normalizing model.
		float scale = 1.f / std::max( std::max( octreeSize.x(), octreeSize.y() ), octreeSize.z() );
		
		#pragma omp parallel for
		for( ulong i = 0; i < m_points->size(); ++i )
		{
			Vec3& pos = ( *m_points )[ i ].getPos();
			pos = ( pos - origin ) * scale;
		}
		
		m_comp.init( Vec3( 0.f, 0.f, 0.f ), octreeSize * scale, leafLvl );
		
		cout << "Normalizing time (ms): " << Profiler::elapsedTime( start ) << endl << endl << "Scale: " << scale << endl << "Octree dim: " <<  m_comp << endl;
	}
	
	template< typename M >
	PointSorter< M >::PointSorter( const string& input, const OctreeDim& dim )
	: m_reader( input ),
	m_comp( dim ),
	m_sortTime( 0u ),
	m_outputTime( 0u )
	{
		cout << "Setup sorting of " << input << endl << endl;
		
		m_points = typename SorterPointSet::PointVectorPtr( new typename SorterPointSet::PointVector( m_reader.getNumPoints() ) );
		
		cout << "Reading " << m_points.size() << " points." << endl << endl;
		auto start = Profiler::now();
		
		long i = 0;
		m_reader.read(
			[ & ]( const Point& p )
			{
				( *m_points )[ i++ ] = p;
			}
		);
		
		m_inputTime = Profiler::elapsedTime( start );
		
		cout << "Reading time (ms): " << m_inputTime << endl << endl;
	}
	
	template< typename M >
	PointSet< M > PointSorter< M >::sort()
	{
		cout << "Starting sort." << endl << endl;
		
		auto start = Profiler::now();
		
		// Sort points.
		
		cout << "Calling std::sort." << endl << endl;
		
		std::sort( m_points->begin(), m_points->end(), m_comp );
		
		m_sortTime = Profiler::elapsedTime( start );
		
		cout << "Sorting time (ms): " << m_sortTime << endl << endl;
		
		return PointSet< M >( m_points, m_comp );
	}
	
	template< typename M >
	Json::Value PointSorter< M >::sortToFile( const string& outFilename )
	{
		sort();
		
		cout << "Start writing. " << endl << endl;
		
		auto start = Profiler::now();
		
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
		
		Writter writter( m_reader, outFilename, m_reader.getNumPoints() );
		
		cout << "Writting output file " << outFilename << endl << endl;
		
		// Write output point file.
		for( long i = 0; i < m_points->size(); ++i )
		{
			writter.write( ( *m_points )[ i ] );
		}
		
		m_outputTime = Profiler::elapsedTime( start );
		
		cout << "Writing time (ms): " << m_outputTime << endl << endl;
		
		return octreeJson;
	}
}

#endif