#ifndef OOC_POINT_SORTER_H
#define OOC_POINT_SORTER_H

#include <jsoncpp/json/json.h>
#include <fstream>
#include <queue>
#include "PlyPointReader.h"
#include "PlyPointWritter.h"
#include <Profiler.h>
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
		
		const OctreeDim& comp() { return m_comp; }
		
	private:
		using PointVector = vector< Point, TbbAllocator< Point > >;
		using ChunkVector = vector< PointVector, TbbAllocator< PointVector > >;
		
		// Min heap entry.
		typedef struct MergeEntry
		{
			using Iter = typename PointVector::iterator;
			
			MergeEntry( PointVector* points, const Iter iter, const int chunkIdx )
			: m_points( points ),
			m_iter( iter ),
			m_chunkIdx( chunkIdx )
			{}
			
			PointVector* m_points;
			Iter m_iter;
			int m_chunkIdx;
		} MergeEntry;
		
		// Min heap comparator.
		typedef struct Comparator
		{
			Comparator( const OctreeDim& dim )
			: m_dim( dim ) {}
			
			bool operator()( const MergeEntry& entry0, const MergeEntry& entry1 ) const
			{
				return m_dim( *entry1.m_iter, *entry0.m_iter );
			}
			
			const OctreeDim& m_dim;
		} Comparator;
		
		using HeapContainer = vector< MergeEntry, TbbAllocator< MergeEntry > >;
		using MinHeap = priority_queue< MergeEntry, HeapContainer, Comparator >;
		
		void writeChunk( PointVector& chunk, typename PointVector::iterator& currentIter, ulong& readPoints, int& nChunks,
						 const Reader& reader ) const;

		PointVector readChunk( const int chunkIdx ) const;
		
		OctreeDim m_comp;
		string m_plyGroupFile;
		string m_plyOutputFolder;
		Vec3 m_origin;
		ulong m_pointsPerChunk;
		ulong m_totalPoints;
		int m_chunksPerMerge;
		float m_scale;
	};
	
	template< typename Morton, typename Point >
	OocPointSorter< Morton,Point >
	::OocPointSorter( const string& plyGroupFile, const string& plyOutputFolder, int lvl, const ulong totalSize,
					  const ulong memoryQuota )
	: m_plyGroupFile( plyGroupFile ),
	m_plyOutputFolder( plyOutputFolder ),
	m_chunksPerMerge( ceil( float( totalSize ) / float( memoryQuota ) ) ),
	m_totalPoints( 0ul )
	{
		if( m_plyGroupFile.find( ".gp" ) == m_plyGroupFile.npos )
		{
			throw runtime_error( "Expected a group file, found: " + m_plyGroupFile );
		}
		
		auto start = Profiler::now( "Boundaries computation" );
		
		Float negInf = -numeric_limits< Float >::max();
		Float posInf = numeric_limits< Float >::max();
		m_origin = Vec3( posInf, posInf, posInf );
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
					++m_totalPoints;
					const Vec3& pos = p.getPos();
					for( int i = 0; i < 3; ++i )
					{
						m_origin[ i ] = glm::min( m_origin[ i ], pos[ i ] );
						maxCoords[ i ] = glm::max( maxCoords[ i ], pos[ i ] );
					}
				}
			);
		}
		
		m_pointsPerChunk = float( m_totalPoints ) / float( m_chunksPerMerge * m_chunksPerMerge );
		
		// Debug
		{
			cout << "Quota: " << memoryQuota << " Chunks: " << m_chunksPerMerge * m_chunksPerMerge
				 << " Points per chunk: " << m_pointsPerChunk << endl << endl;
		}
		
		Vec3 octreeSize = maxCoords - m_origin;
		m_scale = 1.f / std::max( std::max( octreeSize.x(), octreeSize.y() ), octreeSize.z() );
		
		m_comp.init( Vec3( 0.f, 0.f, 0.f ), octreeSize * m_scale, lvl );
		
		// Debug
		{
			cout << "Scale: " << m_scale << endl << "Octree dim: " << m_comp << endl;
		}
		
		Profiler::elapsedTime( start, "Boundaries computation" );
	}
	
	template< typename Morton, typename Point >
	Json::Value OocPointSorter< Morton,Point >::sort()
	{
		omp_set_num_threads( 8 );
		
		string sortedFilename = m_plyOutputFolder + "/sorted.ply";
		
		auto start = Profiler::now( "Chunk sorting" );
		
		int nChunks = 0;
		{
			ulong readPoints = 0;
			ifstream groupFile( m_plyGroupFile );
			
			PointVector chunkPoints( m_pointsPerChunk );
			auto iter = chunkPoints.begin();
			
			// Read, scale and sort chunks.
			while( !groupFile.eof() )
			{
				string plyFilename;
				getline( groupFile, plyFilename );
				
				Reader reader( plyFilename );
				reader.read(
					[ & ]( const Point& p )
					{
						// Send to chunk and scale.
						*iter = p;
						Vec3& pos = iter++->getPos();
						pos = ( pos - m_origin ) * m_scale;
						++readPoints;
						
						if( readPoints == m_pointsPerChunk )
						{
							writeChunk( chunkPoints, iter, readPoints, nChunks, reader );
						}
					}
				);
			}
			
			if( readPoints > 0 )
			{
				// Write leftovers in the last chunk.
				stringstream ss; ss << m_plyOutputFolder << "/sorted_chunk0.ply";
				Reader reader( ss.str() );
				auto iter = chunkPoints.begin();
				writeChunk( chunkPoints, iter, readPoints, nChunks, reader );
			}
		}
		
		Profiler::elapsedTime( start, "Chunk sorting" );
		start = Profiler::now( "Chunk merging" );
		
		{
			// K-way merge chunks
			ChunkVector chunkVector( m_chunksPerMerge );
			Comparator comparator( m_comp );
			HeapContainer heapContainer;
			MinHeap minHeap( comparator, heapContainer );
			int chunkGroupIdx = 0;
			
			// Init the k chunks to start merging.
			for( PointVector& chunk : chunkVector )
			{
				int chunkIdx = chunkGroupIdx++ * m_chunksPerMerge;
				chunk = readChunk( chunkIdx );
				minHeap.push( MergeEntry( &chunk, chunk.begin(), chunkIdx ) );
			}
			
			Writter resultWritter( Reader( m_plyOutputFolder + "/sorted_chunk0.ply" ), sortedFilename, m_totalPoints );
			
			while( !minHeap.empty() )
			{
				MergeEntry entry = minHeap.top();
				
				// Debug
				{
					cout << "top: " << m_comp.calcMorton( *entry.m_iter ).getPathToRoot( true ) << *entry.m_iter << endl;
				}
				
				minHeap.pop();
				
				resultWritter.write( *entry.m_iter );
				
				typename MergeEntry::Iter nextIter = next( entry.m_iter );
				
				if( nextIter == entry.m_points->end() )
				{
					// Get next available chunk.
					int currentChunkGroupIdx = float( ++entry.m_chunkIdx ) / float( m_chunksPerMerge );
					int nextChunkGroupIdx = float( entry.m_chunkIdx ) / float( m_chunksPerMerge );
					if( currentChunkGroupIdx == nextChunkGroupIdx && entry.m_chunkIdx < nChunks )
					{
						*entry.m_points = readChunk( entry.m_chunkIdx );
						entry.m_iter = entry.m_points->begin();
						minHeap.push( entry );
					}
				}
				else
				{
					entry.m_iter = nextIter;
					minHeap.push( entry );
				}
			}
		}
		
		Profiler::elapsedTime( start, "Chunk merging" );
		
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
	
	template< typename Morton, typename Point >
	void OocPointSorter< Morton, Point >
	::writeChunk( PointVector& chunk, typename PointVector::iterator& currentIter, ulong& readPoints, int& nChunks,
				  const Reader& reader ) const 
	{
		// Sort chunk.
		std::sort( chunk.begin(), chunk.end(), m_comp );
		
		// Write to temporary file.
		stringstream ss; ss << m_plyOutputFolder << "/sorted_chunk" << nChunks++ << ".ply";
		
		cout << "Writting chunk with size " << readPoints << " at " << ss.str() << endl << endl;
		
		Writter writter( reader, ss.str(), readPoints );
		
		auto chunkIter = chunk.begin();
		for( int i = 0; i < readPoints; ++i )
		{
			writter.write( *chunkIter++ );
		}
		
		currentIter = chunk.begin();
		readPoints = 0;
	}
	
	template< typename Morton, typename Point >
	typename OocPointSorter< Morton, Point >::PointVector OocPointSorter< Morton, Point >
	::readChunk( const int chunkIdx ) const 
	{
		ulong readPoints = 0;
		PointVector chunk( m_pointsPerChunk );
		auto iter = chunk.begin();
		
		stringstream ss; ss << m_plyOutputFolder << "/sorted_chunk" << chunkIdx << ".ply";
		Reader reader( ss.str() );
		reader.read(
			[ & ]( const Point& p )
			{
				*iter++ = p;
				++readPoints;
			}
		);
		
		if( readPoints < m_pointsPerChunk )
		{
			chunk.resize( readPoints );
		}
		
		return chunk;
	}
}

#endif