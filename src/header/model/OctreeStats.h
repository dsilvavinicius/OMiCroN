#ifndef OCTREE_STATS_H
#define OCTREE_STATS_H

#include <ostream>
#include <Eigen/Dense>
#include "Stream.h"
#include "ReconstructionParams.h"

using namespace std;
using namespace Eigen;

namespace model
{
	/** Statistics of a frame. */
	class FrameStats
	{
	public:
		FrameStats( const float traversalTime = 0.f, const float renderQueueTime = 0.f, const float nRenderedPoints = 0.f,
					const float frontInsertionDelay = 0.f, const float frontSize = 0.f, const float frontSegmentSize = 0.f )
		: m_traversalTime( traversalTime ),
		m_renderQueueTime( renderQueueTime ),
		m_cpuOverhead( traversalTime + renderQueueTime ),
		m_nRenderedPoints( nRenderedPoints ),
		m_frontInsertionDelay( frontInsertionDelay ),
		m_frontSize( frontSize ),
		m_frontSegmentSize( frontSegmentSize )
		{}
		
		friend ostream& operator<<( ostream& out, const FrameStats& frame )
		{
			out << "Traversal time: " << frame.m_traversalTime << endl
				<< "Render queue time: " << frame.m_renderQueueTime << endl
				<< "CPU overhead: " << frame.m_cpuOverhead << endl
				<< "Rendered points: " << frame.m_nRenderedPoints << endl
				<< "Front insertion delay: " << frame.m_frontInsertionDelay << endl
				<< "Front size: " << frame.m_frontSize << endl
				<< "Front segments: " << SEGMENTS_PER_FRONT << endl
				<< "Front segment size: " << frame.m_frontSegmentSize;
			return out;
		}
		
		float m_traversalTime;
		float m_renderQueueTime;
		float m_cpuOverhead;
		float m_nRenderedPoints;
		float m_frontInsertionDelay;
		float m_frontSize;
		float m_frontSegmentSize;
	};
	
	/** Statistics of an Octree. */
	class OctreeStats
	{
	public:
		OctreeStats()
		: m_nFrames( 0.f ),
		m_nFrontInsertions( 0.f )
		{
		}
		
		void addFrame( const FrameStats& frame )
		{
			++m_nFrames;
			
			m_currentStats = frame;
			
			float avgTraversalTime = calcIncrementalAvg( m_currentStats.m_traversalTime, m_avgStats.m_traversalTime, m_nFrames );
			float avgRenderQueueTime =  calcIncrementalAvg( m_currentStats.m_renderQueueTime, m_avgStats.m_renderQueueTime, m_nFrames );
			float avgRenderedPoints = calcIncrementalAvg( m_currentStats.m_nRenderedPoints, m_avgStats.m_nRenderedPoints, m_nFrames );
			float avgFrontInsertionDelay;
			if( m_currentStats.m_frontInsertionDelay > 0.f )
			{
				++m_nFrontInsertions;
				avgFrontInsertionDelay = calcIncrementalAvg( m_currentStats.m_frontInsertionDelay,
															 m_avgStats.m_frontInsertionDelay, m_nFrontInsertions );
			}
			else
			{
				avgFrontInsertionDelay = m_avgStats.m_frontInsertionDelay;
			}
			float avgFrontSize = calcIncrementalAvg( m_currentStats.m_frontSize, m_avgStats.m_frontSize, m_nFrames );
			float avgFrontSegmentSize = calcIncrementalAvg( m_currentStats.m_frontSegmentSize, m_avgStats.m_frontSegmentSize, m_nFrames );
			
			m_avgStats = FrameStats( avgTraversalTime, avgRenderQueueTime, avgRenderedPoints, avgFrontInsertionDelay, avgFrontSize,
									 avgFrontSegmentSize );
		}
		
		float calcIncrementalAvg( const float newValue, const float currentAvg, const float nFrames ) const
		{
			return currentAvg + ( newValue - currentAvg ) / nFrames;
		}
		
		friend ostream& operator<<( ostream& out, const OctreeStats& octreeStats )
		{
			out << "=== CURRENT FRAME STATS ===" << endl << octreeStats.m_currentStats << endl << endl
				<< "=== AVERAGE STATS === " << endl << octreeStats.m_avgStats;
			return out;
		}
		
	public:
		FrameStats m_currentStats;
		FrameStats m_avgStats;
		
		float m_nFrames;
		float m_nFrontInsertions;
	};
	
	/** Cumulus' statistics. */
	class CumulusStats
	{
	public:
		CumulusStats( const float projThresh )
		: m_projThresh( projThresh ),
		m_hierarchyDepth( 0 ),
		m_gpuOverhead( 0.f ),
		m_avgGpuOverhead( 0.f )
		{}
		
		void addFrame( const OctreeStats& octreeStats, const float gpuOverhead )
		{
			m_octreeStats = octreeStats;
			m_gpuOverhead = gpuOverhead;
			m_avgGpuOverhead = octreeStats.calcIncrementalAvg( m_gpuOverhead, m_avgGpuOverhead, octreeStats.m_nFrames );
		}
		
		friend ostream& operator<<( ostream& out, const CumulusStats& cumulusStats )
		{
			out << "Dataset: " << cumulusStats.m_datasetName << endl << endl
				<< "Hierarchy creation threads: " << HIERARCHY_CREATION_THREADS << endl << endl
				<< "Work list size: " << WORK_LIST_SIZE << endl << endl
				<< "GPU memory allowed: " << GPU_MEMORY << endl << endl
				<< "Projection threshold: " << cumulusStats.m_projThresh << endl << endl
				<< "Hierarchy depth: " << cumulusStats.m_hierarchyDepth << endl << endl
				<< "Parent point ratio: " << PARENT_POINTS_RATIO_VALUE << endl << endl
				<< "Leaf collapse: " <<
				#ifdef NODE_COLAPSE
					"true"
				#else
					"false"
				#endif
				<< endl << endl
				<< "Leaf tangent sizes: " << endl << "{ " << LEAF_SURFEL_TANGENT_SIZE_X
				<< ", " << LEAF_SURFEL_TANGENT_SIZE_Y << " }" << endl << endl
				<< "=== Tangent multipliers === " << endl << "Level 1 : " << endl << ReconstructionParams::calcMultipliers( 1 ) << endl
				<< "Level 2 : " << endl << ReconstructionParams::calcMultipliers( 2 ) << endl
				<< "Level 3 : " << endl << ReconstructionParams::calcMultipliers( 3 ) << endl
				<< "Level 4 : " << endl << ReconstructionParams::calcMultipliers( 4 ) << endl
				<< "Level 5 : " << endl << ReconstructionParams::calcMultipliers( 5 ) << endl
				<< "Level 6 : " << endl << ReconstructionParams::calcMultipliers( 6 ) << endl
				<< "Level 7 : " << endl << ReconstructionParams::calcMultipliers( 7 ) << endl
				<< "Level 8 : " << endl << ReconstructionParams::calcMultipliers( 8 ) << endl << endl
				<< "Reconstruction algorithm: " << RECONSTRUCTION_ALG << endl << endl
				<< cumulusStats.m_octreeStats << endl << endl
				<< "=== GPU STATS === " << endl << "GPU Overhead: " << cumulusStats.m_gpuOverhead << endl
				<< "Average GPU Overhead: " << cumulusStats.m_avgGpuOverhead;
			return out;
		}
		
		string m_datasetName;
		int m_workListSize;
		float m_projThresh;
		
		uint m_hierarchyDepth;
		
		OctreeStats m_octreeStats;
		
		float m_gpuOverhead;
		float m_avgGpuOverhead;
	};
}

#endif