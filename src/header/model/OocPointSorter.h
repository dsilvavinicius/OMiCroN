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
		 * @param eraseChunkFiles is true if the temporary chunk files are expected to be deleted after sorting, false
		 * otherwise.
		 * @returns the json written to the resulting octree file. */
		Json::Value sort( bool eraseChunkFilesFlag = true );
		
		/** Erases the chunk files. */
		void eraseChunkFiles();
		
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
		
		void writeChunkGroup( PointVector& chunk, typename PointVector::iterator& currentIter, const ulong& readPoints,
							  int& nChunks, const Reader& reader ) const;

		PointVector readChunk( const int chunkIdx ) const;
		
		OctreeDim m_comp;
		string m_plyGroupFile;
		string m_plyOutputFolder;
		Vec3 m_origin;
		
		ulong m_totalPoints;
		ulong m_pointsPerChunkGroup;
		ulong m_pointsPerChunk;
		int m_chunksPerGroup;
		int m_groups;
		
		float m_scale;
	};
	
	template< typename Morton, typename Point >
	OocPointSorter< Morton,Point >
	::OocPointSorter( const string& plyGroupFile, const string& plyOutputFolder, int lvl, const ulong totalSize,
					  const ulong memoryQuota )
	: m_plyGroupFile( plyGroupFile ),
	m_plyOutputFolder( plyOutputFolder ),
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
						m_origin[ i ] = std::min( m_origin[ i ], pos[ i ] );
						maxCoords[ i ] = std::max( maxCoords[ i ], pos[ i ] );
					}
				}
			);
		}
		
		m_groups =  ceil( float( totalSize ) / float( memoryQuota ) );
		m_pointsPerChunkGroup = ceil( float( m_totalPoints ) / float( m_groups ) );
		m_pointsPerChunk = ceil( float( m_pointsPerChunkGroup ) / float( m_groups ) );
		m_chunksPerGroup = ceil( float( m_pointsPerChunkGroup ) / float( m_pointsPerChunk ) );
		
		// Debug
		{
			cout << "Quota: " << memoryQuota << " Groups: " << m_groups << " Chunks per group: " << m_chunksPerGroup
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
	Json::Value OocPointSorter< Morton,Point >::sort( bool eraseChunkFilesFlag )
	{
		omp_set_num_threads( 8 );
		
		int nameBeginIdx = ( m_plyGroupFile.find_last_of( '/' ) == m_plyGroupFile.npos ) ? 0
							: m_plyGroupFile.find_last_of( '/' ) + 1;
		int nameEndIdx = m_plyGroupFile.find_last_of( '.' );
		string datasetName = m_plyGroupFile.substr( nameBeginIdx, nameEndIdx - nameBeginIdx );
		string sortedFilename = m_plyOutputFolder + "/" + datasetName + ".ply";
		
		auto start = Profiler::now( "Chunk sorting" );
		
		int nChunks = 0;
		{
			ulong readPoints = 0;
			ifstream groupFile( m_plyGroupFile );
			
			PointVector chunkPoints( m_pointsPerChunkGroup );
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
						
						ulong remainder = readPoints % m_pointsPerChunkGroup;
						
						if( remainder == 0 || readPoints == m_totalPoints )
						{
							if( remainder > 0 )
							{
								chunkPoints.resize( remainder );
							}
							writeChunkGroup( chunkPoints, iter, readPoints, nChunks, reader );
						}
					}
				);
			}
		}
		
		// Debug
		{
			cout << "Total chunks: " << nChunks << endl << endl;
		}
		
		Profiler::elapsedTime( start, "Chunk sorting" );
		start = Profiler::now( "Chunk merging" );
		
		{
			// K-way merge chunks
			ChunkVector chunkVector( m_groups );
			Comparator comparator( m_comp );
			HeapContainer heapContainer;
			MinHeap minHeap( comparator, heapContainer );
			int chunkGroupIdx = 0;
			
			// Init the k chunks to start merging.
			for( PointVector& chunk : chunkVector )
			{
				int chunkIdx = chunkGroupIdx++ * m_chunksPerGroup;
				
				if( chunkIdx < nChunks )
				{
					chunk = readChunk( chunkIdx );
					minHeap.push( MergeEntry( &chunk, chunk.begin(), chunkIdx ) );
				}
			}
			
			Writter resultWritter( Reader( m_plyOutputFolder + "/sorted_chunk0.ply" ), sortedFilename, m_totalPoints );
			
			while( !minHeap.empty() )
			{
				MergeEntry entry = minHeap.top();
				minHeap.pop();
				
				resultWritter.write( *entry.m_iter );
				
				typename MergeEntry::Iter nextIter = next( entry.m_iter );
				
				if( nextIter == entry.m_points->end() )
				{
					// Get next available chunk.
					int currentChunkGroupIdx = float( entry.m_chunkIdx++ ) / float( m_chunksPerGroup );
					int nextChunkGroupIdx = float( entry.m_chunkIdx ) / float( m_chunksPerGroup );
					
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
		
		if( eraseChunkFilesFlag )
		{
			eraseChunkFiles();
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
	void OocPointSorter< Morton, Point >::eraseChunkFiles()
	{
		int nChunks = m_groups * m_chunksPerGroup;
		
		// Erase temporary chunk files.
		for( int i = 0; i < nChunks; ++i )
		{
			stringstream ss; ss << m_plyOutputFolder << "/sorted_chunk" << i << ".ply";
			remove( ss.str().c_str() );
		}
	}
	
	template< typename Morton, typename Point >
	inline void OocPointSorter< Morton, Point >
	::writeChunkGroup( PointVector& chunk, typename PointVector::iterator& currentIter, const ulong& readPoints,
					   int& nChunks, const Reader& reader ) const 
	{
		// Sort chunk.
		std::sort( chunk.begin(), chunk.end(), m_comp );
		
		auto chunkIter = chunk.begin();
		
		ulong remainder = readPoints % m_pointsPerChunkGroup;
		ulong pointsLeftInGroup = ( remainder == 0 ) ? m_pointsPerChunkGroup : remainder;
		
		cout << "Writting chunk group with size " << pointsLeftInGroup << endl << endl;
		
		// Write to chunk files.
		while( pointsLeftInGroup > 0 )
		{
			ulong chunkSize = ( pointsLeftInGroup < m_pointsPerChunk ) ? pointsLeftInGroup : m_pointsPerChunk;
			pointsLeftInGroup -= chunkSize;
			
			stringstream ss; ss << m_plyOutputFolder << "/sorted_chunk" << nChunks++ << ".ply";
			cout << "Writting chunk with size " << chunkSize << " at " << ss.str() << endl << endl;
			Writter writter( reader, ss.str(), chunkSize );
			
			for( int i = 0; i < chunkSize; ++i )
			{
				writter.write( *chunkIter++ );
			}
		}
		
		assert( pointsLeftInGroup == 0 && "Not all points where wrote or memory was overrun." );
		
		currentIter = chunk.begin();
	}
	
	template< typename Morton, typename Point >
	inline typename OocPointSorter< Morton, Point >::PointVector OocPointSorter< Morton, Point >
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