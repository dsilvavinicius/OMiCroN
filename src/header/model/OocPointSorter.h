#ifndef OOC_POINT_SORTER_H
#define OOC_POINT_SORTER_H

#include <jsoncpp/json/json.h>
#include <fstream>
#include <queue>
#include "PlyPointReader.h"
#include "PlyPointWritter.h"
#include "OctreeDimensions.h"

using namespace std;
using namespace util;

namespace model
{
	/** Out-of-core point sorter. Uses a simple k-way merge as the external sorting algorithm, dividin the data in k chunks,
	 * applying a first pass where a parallel quicksort sorts each chunk and a second pass where the chunks are k-way merged
	 * as the final result.*/
	template< typename Morton, typename Point >
	class OocPointSorter
	{
	public:
		using Reader = PlyPointReader< Point >;
		using Writter = PlyPointWritter< Point >;
		using OctreeDim = OctreeDimensions< Morton, Point >;
		
		/** Ctor. 
		 @param plyGroupFile is a simple formatted .gp group with all .ply files to be sorted. The format is an absolute
		 absolute path for a .ply file per line.
		 @param plyOutputFolder is the path to the folder where the temporary files and the final sorter file will be.
		 @param lvl is the octree level used to compute morton code for sorting.
		 @param totalSize is the estimation of plyFolder's contents total size. This is used for calculating chunk size.
		 In bytes.
		 @param memoryQuota is the available memory for reading the points. Memory for sorting and writting a chunk should
		 be left. In bytes. */
		OocPointSorter( const string& plyGroupFile, const string& plyOutputFolder, int lvl, const ulong totalSize,
						const ulong memoryQuota );
		
		/** Sorts the points.
		 @returns the json written to the resulting octree file. */
		Json::Value sort();
		
	private:
		// Min heap entry.
		typedef struct MergeEntry
		{
			using Iter = typename PointVector::iterator;
			
			MergeEntry( PointVector* points, const Iter iter )
			: m_points( points ),
			m_iter( iter )
			{}
			
			const PointVector* m_points;
			const Iter m_iter;
		} MergeEntry;
		
		// Min heap comparator.
		typedef struct Comparator
		{
			Comparator( const OctreeDim& dim )
			: m_dim( dim ) {}
			
			bool operator()( const MergeEntry& entry0, const MergeEntry& entry1 )
			{
				return m_dim( *entry1.m_iter, *entry0.m_iter );
			}
			
			const OctreeDim& m_dim;
		} Comparator;
		
		
		OctreeDim m_comp;
		string m_plyGroupFile;
		string m_plyOutputFolder;
		ulong m_pointsPerChunk;
		int m_chunksPerMerge;
		float m_scale;
	};
	
	template< typename Morton, typename Point >
	OocPointSorter< Morton,Point >
	::OocPointSorter( const string& plyGroupFile, const string& plyOutputFolder, int lvl, const ulong totalSize,
					  const ulong memoryQuota )
	: m_plyGroupFile( plyGroupFile ),
	m_plyOutputFolder( plyOutputFolder ),
	m_chunksPerMerge( totalSize / memoryQuota ),
	m_pointsPerChunk( ( memoryQuota / m_chunksPerMerge ) / sizeof( Point ) )
	{
		if( m_plyGroupFile.find( ".gp" ) == m_plyGroupFile.npos )
		{
			throw runtime_error( "Expected a group file, found: " + m_plyGroupFile );
		}
		
		Float negInf = -numeric_limits< Float >::max();
		Float posInf = numeric_limits< Float >::max();
		Vec3 origin = Vec3( posInf, posInf, posInf );
		Vec3 maxCoords( negInf, negInf, negInf );
		
		ifstream groupFile( m_plyGroupFile );
		
		while( !groupFile.eof() )
		{
			string plyFilename;
			getline( groupFile, plyFilename );
			Reader reader( plyFilename );
			reader.read(
				[ & ]( const Point& p )
				{
					const Vec3& pos = p.getPos();
					for( int i = 0; i < 3; ++i )
					{
						origin[ i ] = glm::min( origin[ i ], pos[ i ] );
						maxCoords[ i ] = glm::max( maxCoords[ i ], pos[ i ] );
					}
				}
			);
		}
		
		Vec3 octreeSize = maxCoords - origin;
		float m_scale = 1.f / std::max( std::max( octreeSize.x(), octreeSize.y() ), octreeSize.z() );
		
		m_comp.init( Vec3( 0.f, 0.f, 0.f ), octreeSize * m_scale, lvl );
	}
	
	template< typename Morton, typename Point >
	Json::Value OocPointSorter< Morton,Point >::sort()
	{
		using PointVector = vector< Point, TbbAllocator< Point > >;
		
		string sortedFilename = m_plyOutputFolder + "/sorted.ply";
		
		int nChunks = 0;
		{
			ulong readPoints = 0;
			ifstream groupFile( m_plyGroupFile );
			
			PointVector chunkPoints( m_pointsPerChunk );
			
			// Read, scale and sort chunks.
			while( !groupFile.eof() )
			{
				string plyFilename;
				getline( groupFile, plyFilename );
				Reader reader( plyFilename );
				reader.read(
					[ & ]( const Point& p )
					{
						// Send to chunk.
						chunkPoints[ readPoints ] = p;
						
						// Scale the point.
						Vec3& pos = chunkPoints[ readPoints++ ].getPos();
						pos = ( pos - m_comp.m_origin ) * m_scale;
						
						if( readPoints == m_pointsPerChunk )
						{
							// Sort chunk.
							std::sort( chunkPoints.begin(), chunkPoints.end(), m_comp );
							
							// Write to temporary file.
							stringstream ss; ss << m_plyOutputFolder << "/" << nChunks++ << ".ply";
							Writter writter( reader, ss.str() );
							
							for( const Point& chunkPoint : chunkPoints )
							{
								writter.write( chunkPoint );
							}
							
							readPoints = 0;
						}
					}
				);
			}
		}
		
		{
			// K-way merge chunks
			vector< PointVector, TbbAllocator< PointVector > > chunkVectors( m_chunksPerMerge );
			
			int readChunks = 0;
			
			// Init the k chunks to start merging.
			for( readChunks = 0; readChunks < m_chunksPerMerge; ++readChunks )
			{
				ulong readPoints = 0;
				chunkVectors[ readChunks ].reserve( m_pointsPerChunk );
				stringstream ss; ss << m_plyOutputFolder << "/" << readChunks << ".ply";
				Reader reader( ss.str() );
				reader.read(
					[ & ]( const Point& p )
					{
						chunkVectors[ readChunks ][ readPoints++ ] = p;
					}
				);
				
				if( readPoints < m_pointsPerChunk )
				{
					chunkVectors[ readChunks ].resize( readPoints );
				}
				
				readPoints = 0;
			}
			
			priority_queue< MergeEntry, vector< MergeEntry, TbbAllocator< MergeEntry > >, Comparator >
				minHeap( Comparator( m_comp) );
			
			for( PointVector& points : chunkVectors )
			{
				minHeap.push( MergeEntry( &points, points.end() ) );
			}
			
			Writter resultWritter( Reader( m_plyOutputFolder + "/0.ply" ), sortedFilename );
			
			while( !minHeap.empty() )
			{
				const MergeEntry entry = minHeap.top();
				
				resultWritter.write( *entry.m_iter );
				
				typename MergeEntry::Iter nextIter = next( entry.m_iter );
				
				if( nextIter == entry.m_points->end() )
				{
					// Get next available chunk.
					if( readChunks < nChunks )
					{
						ulong readPoints = 0;
						PointVector chunkPoints( m_pointsPerChunk );
						stringstream ss; ss << m_plyOutputFolder << "/" << readChunks++ << ".ply";
						Reader reader( ss.str() );
						reader.read(
							[ & ]( const Point& p )
							{
								chunkPoints[ readPoints++ ] = p;
							}
						);
						
						if( readPoints < m_pointsPerChunk )
						{
							chunkPoints.resize( readPoints );
						}
						
						*entry.m_points = std::move( chunkPoints );
						
						minHeap.push( MergeEntry( entry.m_points, entry.m_points->begin() ) );
					}
				}
				else
				{
					minHeap.push( MergeEntry ( entry.m_points, nextIter ) );
				}
				
				minHeap.pop();
			}
		}
		
		string dbFilename = sortedFilename.substr( 0, sortedFilename.find_last_of( '.' ) );
		string octreeFilename = dbFilename;
		dbFilename.append( ".db" );
		octreeFilename.append( ".oct" );
		
		Json::Value octreeJson;
		octreeJson[ "points" ] = sortedFilename;
		octreeJson[ "database" ] = dbFilename;
		octreeJson[ "size" ][ "x" ] = m_comp.m_size.x();
		octreeJson[ "size" ][ "y" ] = m_comp.m_size.y();
		octreeJson[ "size" ][ "z" ] = m_comp.m_size.z();
		octreeJson[ "depth" ] = m_comp.m_nodeLvl;
		
		ofstream octreeFile( octreeFilename, ofstream::out );
		octreeFile << octreeJson << endl;
		
		return octreeJson;
	}
}

#endif