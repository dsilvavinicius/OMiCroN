#ifndef POINT_MERGER_H
#define POINT_MERGER_H

#include <queue>
#include "PlyPointReader.h"
#include "PlyFaceReader.h"
#include "PlyPointAndFaceWriter.h"

using namespace util;

namespace model
{
	/** Merges point files into a resulting file. */
	class PlyPointMerger
	{
	public:
		/** Ctor.
		 * @param groupFilename is the path to a file containing the path to the point files, a file per line.
		 * @param outputFilename is the path to the output merge file. */
		PlyPointMerger( const string& groupFilename, const string& outputFilename );
		
		void merge();
	private:
		string m_groupFilename;
		string m_outputFilename;
	};
	
	inline PlyPointMerger::PlyPointMerger( const string& groupFilename, const string& outputFilename )
	: m_groupFilename( groupFilename ),
	m_outputFilename( outputFilename )
	{
		if( m_groupFilename.find( ".gp" ) == m_groupFilename.npos )
		{
			throw runtime_error( "Expected a group file, found: " + m_groupFilename );
		}
	}
	
	inline void PlyPointMerger::merge()
	{
		queue< uint > pointPrefixSum;
		pointPrefixSum.push( 0ul );
		
		uint totalPoints = 0u;
		uint totalFaces = 0u;
		
		// Header pass: compute total number of points, faces and prefix sum for point indices.
		ifstream groupFile( m_groupFilename );
		
		cout << "Starting header pass." << endl << endl;
		
		while( !groupFile.eof() )
		{
			string plyFilename;
			getline( groupFile, plyFilename );
			
			PlyPointReader pointReader( plyFilename );
			PlyFaceReader faceReader( plyFilename, []( const Vec3& face ){} );
			
			totalPoints += pointReader.getNumPoints();
			totalFaces += faceReader.numFaces();
			
			pointPrefixSum.push( totalPoints );
		}
		
		cout << "Header pass. Total points: " << totalPoints << ", total faces: " << totalFaces << endl << endl;
		
		PlyPointAndFaceWriter writer( m_outputFilename, totalPoints, totalFaces );
		
		// First pass: points.
		groupFile = ifstream( m_groupFilename );
		
		while( !groupFile.eof() )
		{
			string plyFilename;
			getline( groupFile, plyFilename );
			
			PlyPointReader pointReader( plyFilename );
			pointReader.read(
				[ & ]( const Point& p )
				{
					writer.writePos( p.getPos() );
				}
			);
		}
		
		cout << "Ended point pass." << endl << endl;
		
		// Second pass: faces.
		groupFile = ifstream( m_groupFilename );
		
		while( !groupFile.eof() )
		{
			string plyFilename;
			getline( groupFile, plyFilename );
			
			PlyFaceReader faceReader( plyFilename,
				[&]( const Vec3& face )
				{
					uint prefixSum = pointPrefixSum.front();
					
					Vec3 prefixedFace( face.x() + prefixSum, face.y() + prefixSum, face.z() + prefixSum );
					writer.writeTri( prefixedFace );
				}
			);
			
			faceReader.read();
			
			pointPrefixSum.pop();
		}
		
		cout << "Ended face pass." << endl << endl;
	}
}

#endif